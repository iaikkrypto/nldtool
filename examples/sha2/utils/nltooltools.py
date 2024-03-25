#!/usr/bin/env python


def printusage(): 
  from sys import stderr
  stderr.write("""\
Usage: 
       ./nltooltools.py found2char <infile> <outdir>
       ./nltooltools.py found2ivchar <infile> <outdir>
       ./nltooltools.py char2tex <infile> <outdir>
       ./nltooltools.py char2tikz <infile> <outdir>
""")

###############################################################################

def chunkify(seq, size):
    return [seq[i:i+size] for i in range(0, len(seq), size)]

def dj(*dicts):
    return dict(sum([d.items() for d in dicts], []))

###############################################################################

def getnewpath(oldfile, newdir, newext):
    paths = oldfile.split("/")
    identifier = ".".join(paths[-1].split(".")[:-1])
    newfile = identifier + "." + newext
    return "/".join(paths[:-2] + [newdir, newfile]), identifier

###############################################################################

def fixxmlnewlines(xmlchar):
    import tempfile
    replace = False
    tmpxml = tempfile.NamedTemporaryFile(delete=False, suffix=".xml", mode="w+t", dir=".")
    with open(xmlchar, "r") as infile:
        for line in infile:
            if line.strip() == "<char value=\"":
                replace = True
            elif replace and line.strip() == "\"/>":
                replace = False
            if replace:
                line = line.rstrip("\n") + "&#10;"
            tmpxml.write(line)
    tmpxml.close()
    return tmpxml.name


def parsechar(xmlchar):
    import xml.etree.ElementTree
    import re
    import os
 
    # Parse file to XML tree
    fixedxmlchar = fixxmlnewlines(xmlchar)
    treeroot = xml.etree.ElementTree.parse(fixedxmlchar).getroot()
    os.unlink(fixedxmlchar)

    # Parse options
    s = 80
    w = 64
    for par in treeroot.find("options"): 
        if par.get("name") == "s": 
            s = int(par.get("value"))
        elif par.get("name") == "w": 
            w = int(par.get("value"))
        elif par.get("name") == "f": 
            if not par.get("value").startswith("sha2"):
                raise "Please provide a SHA-2 characteristic!"

    # Parse characteristic
    char = treeroot.find("char").get("value")
    words = [ss.strip() for ws in re.split(r"\s*\S*?:\s*", char) for ss in ws.split() if len(ss) >= w]
    lines = [l for l in char.split("\n") if len(l) > 1]
    if len(lines) > s+4:
        sortedlines = [lines[i:i+2] for i in range(0,len(lines), 2)]
        mergedlines = "".join(["".join(["T" if c == "-" and c2 != " " else c for c, c2 in zip(cond, twobit)]) for cond, twobit in sortedlines])
        words = [ss.strip() for ws in re.split(r"\s*\S*?:\s*", mergedlines) for ss in ws.split() if len(ss) >= w]
    assert len(words) == 3*s + 2*4
    return chunkify(words[:8], 2) + chunkify(words[8:], 3)


###############################################################################

def writexmlhead(outfile, f, s, w):
    outfile.write("""\
<config>
<options>
  <option name="f"  value="{f}"/>
  <option name="n"  value="{s}"/>
  <option name="w"  value="{w}"/>
  <option name="z"  value="main"/>
</options>
<char value="
""".format(**locals()))

def writexml(outfile, states, offset=-4, names="AEW", f="sha2lin"):
    w = len(states[0][0])
    s = len(states) + offset
    writexmlhead(outfile, f, s, w)
    outfile.write("\n".join([
        "{i:3d} ".format(i=i+offset) + " ".join([
            "{name}: {value}".format(**locals())
            for name, value in zip(names, sline)])
        for i, sline in enumerate(states)]))
    writexmlend(outfile)

def writexmlend(outfile):
    outfile.write("""\
"/>
</config>
""")

def writeprintcfg(twobit=0):
    with open("printconfig.xml", "w") as cfgxml:
        cfgxml.write("""\
<printconfig>
  <flags>
    <flag name="color" value="0"/>
    <flag name="twobit" value="{twobit}"/>
  </flags>
</printconfig>\
""".format(**locals()))


###############################################################################

def writetexhead(outfile):
    outfile.write("""\
\\documentclass[a4paper]{article}
\\usepackage[utf8]{inputenc}
\\usepackage{xcolor}
\\usepackage{graphicx}
\\usepackage{rotating} 
\\usepackage[margin=1cm]{geometry}

\\begin{document}
\\pagestyle{empty}
""")

def writetextable(outfile, states, UN01defs, caption="Characteristic for SHA-2", label="tab:char", tableheads="AEW", offset=-4):
    w = len(states[0][0])
    cols = max([len(line) for line in states])
    tabletype = "table" if w <= 32 else "sidewaystable"
    tabspec = "|r|" + cols*"c|"
    header = "$i$    & " + " & ".join(["{{h:{l}}}".format(l=9+2*w).format(h="$"+h+"_i$") for h in tableheads[:cols]]) + "\\\\"
    def padhack(line):
        return (line + [w * " "])[:3]
    outfile.write("""
{{
\\definecolor{{gray}}{{rgb}}{{.7,.7,.7}}
""".format() + "".join(["""\
\\newcommand{{\\{L}}}{{{macro}}}
""".format(**locals()) for L, macro in UN01defs.values()]) + """
\\begin{{{tabletype}}}
\\caption{{{caption}}}
\\label{{{label}}}
\\centering
\\scalebox{{.55}}{{
\\begin{{tabular}}{{{tabspec}}}
\\hline
{header}
\\hline
""".format(**locals()) + "\n".join([
    "${i:3}$ & {cols} \\\\".format(i=i+offset, cols=" & ".join([
        "\\texttt{{{fword}}}".format(fword="".join(["\\{L}".format(L=UN01defs[c][0]) for c in word]))
        #"{{\\tt{fword}}}".format(fword="".join(["\\{L}".format(L=UN01defs[c][0]) for c in word]))
        for word in padhack(line)
        ]))
    for i, line in enumerate(states)]) + """
\\hline
\\end{{tabular}}
}}
\\end{{{tabletype}}}
}}
""".format(**locals()))


def writetexend(outfile):
    outfile.write("""
\\end{document}
""")

###############################################################################

def found2char(inchar, outfile, keepiv=4, keepconditions=True, twobit=True):
    import os
    import tempfile
    def f2ch(l):
        return "-" if l in "01" else l
    charstate = inchar[:keepiv] + [["".join(f2ch(l) for l in word) for word in line] for line in inchar[keepiv:]]
    if not keepconditions:
        writexml(outfile, charstate)
    else:
        tmpxml = tempfile.NamedTemporaryFile(delete=False, suffix=".xml", mode="w+t", dir=".")
        tmplog = tempfile.NamedTemporaryFile(delete=False, suffix=".log", mode="w+t", dir=".")
        writexml(tmpxml, charstate)
        tmpxml.close()
        tmplog.close()
        tmpxmlname = tmpxml.name
        tmplogname = tmplog.name
        writeprintcfg(int(twobit))
        os.system("./nltool -u checkchar -i {tmpxmlname} -l {tmplogname} -c 2".format(**locals()))
        os.unlink("./printconfig.xml")
        with open(tmplogname) as tmplogfile:
            prefix = True
            for line in tmplogfile:
                if prefix:
                    if line.startswith("Info: computing 2-bit conditions"):  # update"):
                        prefix = False
                        writexmlhead(outfile, f="sha2lin", s=len(charstate)-4, w=len(charstate[0][0]))
                else:
                    if line.startswith("Info: complete on 2-bit conditions"): # update"):
                        writexmlend(outfile)
                    else:
                        outfile.write(line)
        os.unlink(tmpxmlname)
        os.unlink(tmplogname)

###############################################################################

def char2tex(inchar, outfile, identifier="SHA-2", color=True, blocks=False, includehead=True):
    conds = "xun-?01T "
    cmdnames = dict(zip(conds, "XUNDQZETW"))
    colors = {} if not color else dj({x: "red" for x in "xun"}, {"T": "gray!50"}) if not blocks else dj({x: "red" for x in "xun"}, {"?": "orange"}, {c: "gray" for c in "01T"}, {"-": "white"})
    def colorwrap(x, color):
        if x == "T":
            return "{{\\colorbox{{{color}}}{{\\hspace{{-3pt}}{{-}}\\hspace{{-3pt}}}}}}".format(**locals()) if color else x
        else:
            return "\\textcolor{{{color}}}{{{x}}}".format(**locals()) if color else x
    def blockcolorwrap(x, color):
        return "{{\\colorbox{{{color}}}{{\\hspace{{-3pt}}{x}\\hspace{{-3pt}}}}}}".format(x=colorwrap(x.upper(), color), color=color) if blocks and color else colorwrap(x, color)
    UN01defs = {x : (cmdnames[x], blockcolorwrap(x, colors.get(x))) for x in conds}
    caption = "Characteristic for {id}".format(id=identifier.replace("_", " "))
    label = "tab:{id}".format(id=identifier.replace("_", "-"))

    if includehead: writetexhead(outfile)
    writetextable(outfile, inchar, UN01defs, caption, label)
    if includehead: writetexend(outfile)

###############################################################################

if __name__ == "__main__":
    from sys import argv
    if len(argv) < 4: 
        printusage()
        exit(1)
    fun = argv[1].strip()
    infile = argv[2].strip()
    outdir = argv[3].strip()
    inchar = parsechar(infile)
    newext = "xml" if fun in ["found2char", "found2ivchar"] else "tex"
    newfile, identifier = getnewpath(infile, outdir, newext)
    with open(newfile, "w") as outfile:
        if fun == "found2char":
            found2char(inchar, outfile, keepiv=0)
        elif fun == "found2ivchar":
            found2char(inchar, outfile, keepiv=4)
        elif fun == "char2tex":
            char2tex(inchar, outfile, identifier)
        elif fun == "char2tikz":
            char2tex(inchar, outfile, identifier, blocks=True)
        else:
            printusage()


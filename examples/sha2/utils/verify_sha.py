#!/usr/bin/env python

### SHA-2 BASE (FROM SLOWSHA) ##################################################
# using code from Stefano Palazzo: https://code.google.com/p/slowsha/

class sha2_32 (object):
    ''' Superclass for both 32 bit SHA2 objects (SHA224 and SHA256) '''

    def __init__(self, message, rounds=64, pad=True):
        length = bin(len(message) * 8)[2:].rjust(64, "0")
        while len(message) > 64:
            self._handle(''.join(bin(ord(i))[2:].rjust(8, "0")
                for i in message[:64]), rounds)
            message = message[64:]
        message = ''.join(bin(ord(i))[2:].rjust(8, "0") for i in message)
        if pad: 
            message += "1"
            message += "0" * ((448 - len(message) % 512) % 512) + length
        else: 
            assert len(message) == 512
        for i in range(len(message) // 512):
            self._handle(message[i * 512:i * 512 + 512], rounds)

    def _handle(self, chunk, rounds):
        assert 16 <= rounds <= 64

        rrot = lambda x, n: (x >> n) | (x << (32 - n))
        w = []

        k = [
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
            0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
            0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
            0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
            0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
            0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
            0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
            0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
            0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
            0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
            0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
            0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
            0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
            0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
            0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2]

        for j in range(len(chunk) // 32):
            w.append(int(chunk[j * 32:j * 32 + 32], 2))

        for i in range(16, rounds):
            s0 = rrot(w[i - 15], 7) ^ rrot(w[i - 15], 18) ^ (w[i - 15] >> 3)
            s1 = rrot(w[i - 2], 17) ^ rrot(w[i - 2], 19) ^ (w[i - 2] >> 10)
            w.append((w[i - 16] + s0 + w[i - 7] + s1) & 0xffffffff)

        a = self._h0
        b = self._h1
        c = self._h2
        d = self._h3
        e = self._h4
        f = self._h5
        g = self._h6
        h = self._h7

        for i in range(rounds):
            s0 = rrot(a, 2) ^ rrot(a, 13) ^ rrot(a, 22)
            maj = (a & b) ^ (a & c) ^ (b & c)
            t2 = s0 + maj
            s1 = rrot(e, 6) ^ rrot(e, 11) ^ rrot(e, 25)
            ch = (e & f) ^ ((~ e) & g)
            t1 = h + s1 + ch + k[i] + w[i]

            h = g
            g = f
            f = e
            e = (d + t1) & 0xffffffff
            d = c
            c = b
            b = a
            a = (t1 + t2) & 0xffffffff

        self._h0 = (self._h0 + a) & 0xffffffff
        self._h1 = (self._h1 + b) & 0xffffffff
        self._h2 = (self._h2 + c) & 0xffffffff
        self._h3 = (self._h3 + d) & 0xffffffff
        self._h4 = (self._h4 + e) & 0xffffffff
        self._h5 = (self._h5 + f) & 0xffffffff
        self._h6 = (self._h6 + g) & 0xffffffff
        self._h7 = (self._h7 + h) & 0xffffffff

    def hexdigest(self):
        return ''.join(hex(i)[2:].rjust(8, "0")
                for i in self._digest())[:(self._trunc/4)]

    def digest(self):
        hexdigest = self.hexdigest()
        return bytes(int(hexdigest[i * 2:i * 2 + 2], 16)
                for i in range(len(hexdigest) // 2))


class sha2_64 (object):
    ''' Superclass for both 64 bit SHA2 objects (SHA384 and SHA512) '''

    def __init__(self, message, rounds=80, pad=True):
        length = bin(len(message) * 8)[2:].rjust(128, "0")
        while len(message) > 128:
            self._handle(''.join(bin(ord(i))[2:].rjust(8, "0")
                for i in message[:128]), rounds)
            message = message[128:]
        message = ''.join(bin(ord(i))[2:].rjust(8, "0") for i in message)
        if pad:
            message += "1"
            message += "0" * ((896 - len(message) % 1024) % 1024) + length
        else:
            assert len(message) == 1024
        for i in range(len(message) // 1024):
            self._handle(message[i * 1024:i * 1024 + 1024], rounds)

    def _handle(self, chunk, rounds):
        assert 16 <= rounds <= 80

        rrot = lambda x, n: (x >> n) | (x << (64 - n))
        w = []

        k = [
            0x428a2f98d728ae22, 0x7137449123ef65cd,
            0xb5c0fbcfec4d3b2f, 0xe9b5dba58189dbbc,
            0x3956c25bf348b538, 0x59f111f1b605d019,
            0x923f82a4af194f9b, 0xab1c5ed5da6d8118,
            0xd807aa98a3030242, 0x12835b0145706fbe,
            0x243185be4ee4b28c, 0x550c7dc3d5ffb4e2,
            0x72be5d74f27b896f, 0x80deb1fe3b1696b1,
            0x9bdc06a725c71235, 0xc19bf174cf692694,
            0xe49b69c19ef14ad2, 0xefbe4786384f25e3,
            0x0fc19dc68b8cd5b5, 0x240ca1cc77ac9c65,
            0x2de92c6f592b0275, 0x4a7484aa6ea6e483,
            0x5cb0a9dcbd41fbd4, 0x76f988da831153b5,
            0x983e5152ee66dfab, 0xa831c66d2db43210,
            0xb00327c898fb213f, 0xbf597fc7beef0ee4,
            0xc6e00bf33da88fc2, 0xd5a79147930aa725,
            0x06ca6351e003826f, 0x142929670a0e6e70,
            0x27b70a8546d22ffc, 0x2e1b21385c26c926,
            0x4d2c6dfc5ac42aed, 0x53380d139d95b3df,
            0x650a73548baf63de, 0x766a0abb3c77b2a8,
            0x81c2c92e47edaee6, 0x92722c851482353b,
            0xa2bfe8a14cf10364, 0xa81a664bbc423001,
            0xc24b8b70d0f89791, 0xc76c51a30654be30,
            0xd192e819d6ef5218, 0xd69906245565a910,
            0xf40e35855771202a, 0x106aa07032bbd1b8,
            0x19a4c116b8d2d0c8, 0x1e376c085141ab53,
            0x2748774cdf8eeb99, 0x34b0bcb5e19b48a8,
            0x391c0cb3c5c95a63, 0x4ed8aa4ae3418acb,
            0x5b9cca4f7763e373, 0x682e6ff3d6b2b8a3,
            0x748f82ee5defb2fc, 0x78a5636f43172f60,
            0x84c87814a1f0ab72, 0x8cc702081a6439ec,
            0x90befffa23631e28, 0xa4506cebde82bde9,
            0xbef9a3f7b2c67915, 0xc67178f2e372532b,
            0xca273eceea26619c, 0xd186b8c721c0c207,
            0xeada7dd6cde0eb1e, 0xf57d4f7fee6ed178,
            0x06f067aa72176fba, 0x0a637dc5a2c898a6,
            0x113f9804bef90dae, 0x1b710b35131c471b,
            0x28db77f523047d84, 0x32caab7b40c72493,
            0x3c9ebe0a15c9bebc, 0x431d67c49c100d4c,
            0x4cc5d4becb3e42b6, 0x597f299cfc657e2a,
            0x5fcb6fab3ad6faec, 0x6c44198c4a475817]

        for j in range(len(chunk) // 64):
            w.append(int(chunk[j * 64:j * 64 + 64], 2))

        for i in range(16, rounds):
            s0 = rrot(w[i - 15], 1) ^ rrot(w[i - 15], 8) ^ (w[i - 15] >> 7)
            s1 = rrot(w[i - 2], 19) ^ rrot(w[i - 2], 61) ^ (w[i - 2] >> 6)
            w.append((w[i - 16] + s0 + w[i - 7] + s1) & 0xffffffffffffffff)

        a = self._h0
        b = self._h1
        c = self._h2
        d = self._h3
        e = self._h4
        f = self._h5
        g = self._h6
        h = self._h7

        for i in range(rounds):
            s0 = rrot(a, 28) ^ rrot(a, 34) ^ rrot(a, 39)
            maj = (a & b) ^ (a & c) ^ (b & c)
            t2 = s0 + maj
            s1 = rrot(e, 14) ^ rrot(e, 18) ^ rrot(e, 41)
            ch = (e & f) ^ ((~ e) & g)
            t1 = h + s1 + ch + k[i] + w[i]

            h = g
            g = f
            f = e
            e = (d + t1) & 0xffffffffffffffff
            d = c
            c = b
            b = a
            a = (t1 + t2) & 0xffffffffffffffff

        self._h0 = (self._h0 + a) & 0xffffffffffffffff
        self._h1 = (self._h1 + b) & 0xffffffffffffffff
        self._h2 = (self._h2 + c) & 0xffffffffffffffff
        self._h3 = (self._h3 + d) & 0xffffffffffffffff
        self._h4 = (self._h4 + e) & 0xffffffffffffffff
        self._h5 = (self._h5 + f) & 0xffffffffffffffff
        self._h6 = (self._h6 + g) & 0xffffffffffffffff
        self._h7 = (self._h7 + h) & 0xffffffffffffffff

    def hexdigest(self):
        return ''.join(hex(i)[2:].rstrip("L").rjust(16, "0")
                for i in self._digest())[:(self._trunc/4)]

    def digest(self):
        hexdigest = self.hexdigest()
        return bytes(int(hexdigest[i * 2:i * 2 + 2], 16)
                for i in range(len(hexdigest) // 2))


### SHA-2 TRUNCATED VARIANTS ###################################################

def build_SHA_32(hashbitlen=256, iv=(
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
        )):
    assert len(iv) == 8
    assert hashbitlen % 32 == 0
    wordlen = hashbitlen/32
    class SHA_32(sha2_32):
        _h0, _h1, _h2, _h3, _h4, _h5, _h6, _h7 = iv
        _trunc = hashbitlen
        def _digest(self):
            return (self._h0, self._h1, self._h2, self._h3,
                    self._h4, self._h5, self._h6, self._h7)[:wordlen]
    return SHA_32

def build_SHA_64(hashbitlen=512, iv=(
        0x6a09e667f3bcc908, 0xbb67ae8584caa73b, 0x3c6ef372fe94f82b, 0xa54ff53a5f1d36f1,
        0x510e527fade682d1, 0x9b05688c2b3e6c1f, 0x1f83d9abfb41bd6b, 0x5be0cd19137e2179
        )):
    assert len(iv) == 8
    assert hashbitlen % 32 == 0
    wordlen = (hashbitlen+63)/64
    lastmask = 0xffffffffffffffff if not hashbitlen - 64*wordlen else 0xffffffff00000000
    class SHA_64(sha2_64):
        _h0, _h1, _h2, _h3, _h4, _h5, _h6, _h7 = iv
        _trunc = hashbitlen
        def _digest(self):
            digest = [self._h0, self._h1, self._h2, self._h3,
                      self._h4, self._h5, self._h6, self._h7][:wordlen]
            digest[-1] &= lastmask
            return tuple(digest)
    return SHA_64

SHA256 = build_SHA_32()

SHA224 = build_SHA_32(224, (
        0xc1059ed8, 0x367cd507, 0x3070dd17, 0xf70e5939,
        0xffc00b31, 0x68581511, 0x64f98fa7, 0xbefa4fa4
        ))

SHA512 = build_SHA_64()

SHA384 = build_SHA_64(384, (
        0xcbbb9d5dc1059ed8, 0x629a292a367cd507, 0x9159015a3070dd17, 0x152fecd8f70e5939,
        0x67332667ffc00b31, 0x8eb44a8768581511, 0xdb0c2e0d64f98fa7, 0x47b5481dbefa4fa4
        ))

SHA512_224 = build_SHA_64(224, (
        0x8c3d37c819544da2, 0x73e1996689dcd4d6, 0x1dfab7ae32ff9c82, 0x679dd514582f9fcf,
        0x0f6d2b697bd44da8, 0x77e36f7304c48942, 0x3f9d85a86a1d36c8, 0x1112e6ad91d692a1
        ))

SHA512_256 = build_SHA_64(256, (
        0x22312194fc2bf72c, 0x9f555fa3c84c64c2, 0x2393b86b6f53b151, 0x963877195940eabd,
        0x96283ee2a88effe3, 0xbe5e1e2553863992, 0x2b0199fc2c85b8aa, 0x0eb72ddc81c52ca2
        ))


### VERIFICATION OF XML FILES ##################################################

def test_this():
    import os
    import hashlib
    vectors = [
        b'',
        b'abc',
        b'The quick brown fox jumped over the lazy dog',
        b'The quick brown fox jumped over the lazy dog.',
        b"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
        os.urandom(1200),
    ]
    for i in vectors:
        assert hashlib.sha224(i).hexdigest() == SHA224(i).hexdigest()
        assert hashlib.sha256(i).hexdigest() == SHA256(i).hexdigest()
        assert hashlib.sha384(i).hexdigest() == SHA384(i).hexdigest()
        assert hashlib.sha512(i).hexdigest() == SHA512(i).hexdigest()
    # examples from  http://csrc.nist.gov/groups/ST/toolkit/examples.html#aHashing
    # and http://en.wikipedia.org/wiki/SHA-2#Examples_of_SHA-2_variants
    assert SHA512_224("").hexdigest() == "6ed0dd02806fa89e25de060c19d3ac86cabb87d6a0ddd05c333b84f4"
    assert SHA512_224("abc").hexdigest() == "4634270f707b6a54daae7530460842e20e37ed265ceee9a43e8924aa"
    assert SHA512_256("").hexdigest() == "c672b8d1ef56ed28ab87c3622c5114069bdd3ad7b8f9737498d0c01ecef0967a"
    assert SHA512_256("abc").hexdigest() == "53048e2681941ef99b2e29b76b4c7dabe4c2d0c634fc6d46e0e2f13107e7af23"

    print "successfully verified current code"


def parsechar(xmlchar):
    import xml.etree.ElementTree
    import re
    treeroot = xml.etree.ElementTree.parse(xmlchar).getroot()
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
    char = treeroot.find("char").get("value")
    words = [ss.strip() for ws in re.split(r"\s*\S*?:\s*", char) for ss in ws.split() if len(ss) >= w]
    assert len(words) == 3*s + 2*4
    def chunkify(seq, size):
        return [seq[i:i+size] for i in range(0, len(seq), size)]
    return chunkify(words[:8], 2) + chunkify(words[8:], 3)


def printhextable(iv_int, msg_bytes, h_hex, bitsperword, colltype, rounds):
    def hex_chunkify(hex, bits):
        return [hex[i:i+(bits/4)] for i in range(0, len(hex), bits/4)]
    iv_diff = [a ^ b for a, b in zip(iv_int[0], iv_int[1])]
    iv_hexs = [[int2hex(i, bitsperword) for i in iv] for iv in iv_int + [iv_diff]]
    msg_diff = "".join([int2bytes(ord(a) ^ ord(b), 8) for a, b in zip(msg_bytes[0], msg_bytes[1])])
    msg_hexs = [hex_chunkify(msg.encode('hex'), bitsperword) for msg in msg_bytes + [msg_diff]]
    hash_hexs = [hex_chunkify(h, bitsperword) for h in h_hex]
    hashbitlen = len(h_hex[0]) * 4
    hashfun = ("SHA-512/{hashbitlen}" if bitsperword == 64 and hashbitlen not in [384, 512] else "SHA-{hashbitlen}").format(**locals())
    identifier = hashfun + "_" + str(rounds)
    ncols = 256/bitsperword
    coltype = "|c|" + ncols*"l" + "|"
    def tablify(words, name):
        if not words: return ""
        rowwise = [words[i:i+ncols] for i in range(0,len(words), ncols)]
        nrows = len(rowwise)
        return "\\multirow{{{nrows}}}{{*}}{{${name}$}}\n".format(**locals()) + "".join([
            "& " + " & ".join(row + [(bitsperword/4)*" " for i in range(ncols - len(row))]) + " \\\\\n" for row in rowwise
            ])
    rows = []
    if colltype != "":
        rows.append((iv_hexs[0], "h_0"))
        if colltype == "free-start ":
            rows.append((iv_hexs[1], "h_0^*"))
            rows.append((iv_hexs[2], "\\Delta h_0"))
        rows.append(([], ""))
    rows.append((msg_hexs[0], "m"))
    rows.append((msg_hexs[1], "m^*"))
    rows.append((msg_hexs[2], "\\Delta m"))
    rows.append(([], ""))
    rows.append((hash_hexs[0], "h_1"))
    rowcode = "\\hline\n" + "\\hline\n".join([tablify(words, name) for words, name in rows]) + "\\hline\n"
    print """
\\begin{{table}}
\\caption{{Example of a {colltype}collision for {rounds} rounds of {hashfun}.}}
\\label{{tab:{identifier}}}
\\centering
\\texttt{{
\\begin{{tabular}}{{{coltype}}}
{rowcode}\
\\end{{tabular}}
}}
\\end{{table}}
""".format(**locals())


def diff2int(diff):
    assert all([b in "01nu" for b in diff])
    ints = [0, 0]
    for b in diff:
        ints[0] = (ints[0] << 1) ^ int(b in ["1", "u"])
        ints[1] = (ints[1] << 1) ^ int(b in ["1", "n"])
    return tuple(ints)


def int2hex(i, bitlen):
    return hex(i)[2:].rstrip("L").rjust(bitlen/4, "0")


def int2bytes(i, bitlen):
    return int2hex(i, bitlen).decode("hex")


def diff2bytes(diff):
    return tuple([int2bytes(i, len(diff)) for i in diff2int(diff)])


def get_sha_version(wordsize, digestsize, fixediv=True, iv=[]):
    shaiv = {
            32: {256: SHA256, 224: SHA224},
            64: {512: SHA512, 384: SHA384, 256: SHA512_256, 224: SHA512_224}
            }
    shabuilder = build_SHA_32 if wordsize == 32 else build_SHA_64
    return shaiv[wordsize][digestsize] if fixediv else shabuilder(digestsize, iv)


def get_sha_name(wordsize, digestsize):
    return ("SHA-512/{digestsize}" if wordsize == 64 and digestsize not in [384, 512] else "SHA-{digestsize}").format(**locals())


def test_xml(xmlchar, hashbitlen, fixediv):
    inchar = parsechar(xmlchar)
    wordlen = len(inchar[0][0])
    rounds = len(inchar) - 4
    pad = False
    ivdiff = (inchar[3][0], inchar[2][0], inchar[1][0], inchar[0][0], 
              inchar[3][1], inchar[2][1], inchar[1][1], inchar[0][1])
    msgdiff = tuple([line[2] for line in inchar[4:20]])
    iv = zip(*[diff2int(d) for d in ivdiff])
    msg = ["".join(wordlist) for wordlist in zip(*[diff2bytes(d) for d in msgdiff])]
    shaversion = [get_sha_version(wordlen, hashbitlen, fixediv, ivw) for ivw in iv]
    h = [shav(msgw, rounds, pad=False).hexdigest() for shav, msgw in zip(shaversion, msg)]
    print "\n".join(h)
    assert h[0] == h[1]
    colltype = "" if fixediv else "semi-free-start " if iv[0] == iv[1] else "free-start "
    printhextable(iv, msg, h, wordlen, colltype, rounds)


def test_cvc(cvcfile, hashbitlen, fixediv):
    with open(cvcfile, "r") as words:
        message_hex = ["" for i in range(16)]
        iv_hex = ["" for i in range(8)]
        nrounds = 0
        for line in words:
            if line.startswith("ASSERT( r") and line[11:16] == "_W = ":
                nrounds = max(nrounds, int(line[9:11])+1)
            if any([line.startswith("ASSERT( r{rr:02d}_W = 0x".format(**locals())) for rr in range(16)]):
                message_hex[int(line[9:11])] = line[18:-4]
            elif any([line.startswith("ASSERT( r_{r}_{AE}_IV = 0x".format(**locals())) for r in range(1, 5) for AE in "AE"]):
                idx = (0 if line[12] == "A" else 4) + int(line[10]) - 1
                iv_hex[idx] = line[21:-4]
        wordsize = len(message_hex[0])*4
        assert wordsize in [32, 64] and all([len(msg) == wordsize/4 for msg in message_hex])
        assert len(iv_hex[0]) == wordsize/4 and all([len(iv) == len(iv_hex[0]) for iv in iv_hex])
        sha = get_sha_version(wordsize, hashbitlen, fixediv, [int(iv, 16) for iv in iv_hex])
        shaname = get_sha_name(wordsize, hashbitlen)
        msg = "".join([w.decode("hex") for w in message_hex])
        h = sha(msg, nrounds, pad=False).hexdigest()
        print "message:", msg.encode("hex")
        print "digest: ", h
        print "(after {nrounds} rounds of {shaname})".format(**locals())


if __name__ == '__main__':
    import sys
    test_this()
    if len(sys.argv) <= 3:
        print "Usage: ./verify_sha.py 224 iv my_sha512_224_coll.xml    (for xml collisions)"
        print "       ./verify_sha.py 224 free my_sha512_224_coll.xml  (for xml (semi-)free-start collisions)"
        print "       ./verify_sha.py 224 pre my_sha512_224_preimg.cvc (for cvc preimages)"
        exit(1)
    digestsize = int(sys.argv[1])
    attack = sys.argv[2]
    infile = sys.argv[3]
    if attack == "pre":
        test_cvc(infile, digestsize, True)
    else:
        test_xml(infile, digestsize, attack=="iv")

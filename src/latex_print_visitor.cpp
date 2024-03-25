#include "latex_print_visitor.h"

#include <iomanip>
#include <sstream>

#include "characteristic.h"
#include "print_matrix.h"

void LatexPrintVisitor::visit_PrintMatrix(const PrintMatrix& printMatrix) {
  PrintLatexTableStartDeclaration();
  PrintVisitor::visit_PrintMatrix(printMatrix);
  PrintLatexTableEndDeclaration();
}

void LatexPrintVisitor::PrintNumber(uint32_t number, int /*r*/, int /*c*/,
                                    int /*color*/, bool box,
                                    bool /*inverse  = false*/) {
  if (box) fs_ << "\\TB{";

  fs_ << (char)(number < 10 ? ('0' + number)
                            : number < 36 ? ('a' + number - 10) : '*');

  if (box) fs_ << "}";
}

void LatexPrintVisitor::PrintBitcondition(uint8_t bc, int /*r*/, int /*c*/,
                                          int /*color*/, bool box,
                                          bool /*inverse = false*/) {
  static unsigned char chars[] = "#0u3n5x71-ABCDE?";
  assert(static_cast<uint8_t>(bc) < 16);

  if (box) fs_ << "\\T{";

  if (colored_) {
    switch (bc) {
      case 0:
        fs_ << "\\#";
        break;
      case 2:
        fs_ << "\\U";
        break;
      case 4:
        fs_ << "\\N";
        break;
      case 6:
        fs_ << "\\X";
        break;
      default:
        fs_ << chars[bc];
        break;
    }
  } else
    fs_ << chars[bc];

  if (box) fs_ << "}";
}

void LatexPrintVisitor::PrintColorCode(uint8_t /*color*/,
                                       bool /*inverse*/) const {
  // do not do color for latex
}

void LatexPrintVisitor::visit_PrintableTwobitDegree(
    const PrintableTwobitDegree& twoBitDegree) {
  PrintLatexWordStart();
  PrintVisitor::visit_PrintableTwobitDegree(twoBitDegree);
  PrintLatexWordEnd();
}

void LatexPrintVisitor::visit_PrintableWord(const PrintableWord& word) {
  PrintLatexWordStart();
  PrintVisitor::visit_PrintableWord(word);
  PrintLatexWordEnd();
}

void LatexPrintVisitor::PrintLatexTableStartDeclaration() const {
  fs_ << std::endl;
  fs_ << "{" << std::endl;
  fs_ << "\\definecolor{gray}{rgb}{0.7,0.7,0.7}" << std::endl;
  fs_ << "\\newcommand{\\X}{{\\color{red}{x}}}" << std::endl;
  fs_ << "\\newcommand{\\U}{{\\color{red}{u}}}" << std::endl;
  fs_ << "\\newcommand{\\N}{{\\color{red}{n}}}" << std::endl;
  fs_ << "\\newcommand{\\T}[1]{\\colorbox{gray}{\\hspace{-3pt}#1\\hspace{-3pt}}"
         "}"
      << std::endl;
  fs_ << std::endl;
  fs_ << "\\begin{table}" << std::endl;
  fs_ << "\\scalebox{.55}{" << std::endl;
  fs_ << "\\begin{tabular}{" << orientation_ << "}" << std::endl;
  fs_ << "\\hline" << std::endl;
  fs_ << "  " << titles_ << " \\\\" << std::endl;
  fs_ << "\\hline" << std::endl;
}

void LatexPrintVisitor::PrintLatexTableEndDeclaration() const {
  fs_ << "\\hline" << std::endl;
  fs_ << "\\end{tabular}" << std::endl;
  fs_ << "}" << std::endl;
  fs_ << "\\end{table}" << std::endl;
  fs_ << "}" << std::endl;
}

void LatexPrintVisitor::PrintLatexWordStart() const { fs_ << "{\\tt "; }

void LatexPrintVisitor::PrintLatexWordEnd() const { fs_ << "}"; }

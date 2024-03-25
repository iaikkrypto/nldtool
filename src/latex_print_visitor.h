#ifndef LATEX_PRINT_VISITOR_H_
#define LATEX_PRINT_VISITOR_H_

#include <cstdint>
#include <iostream>

#include "print_visitor.h"

class PrintMatrix;
class PrintableSpace;
class PrintableString;
class PrintableNumber;
class PrintableTwobitDegree;
class PrintableWord;
class PrintableProbability;
class PrintableProbabilitySum;

/*!
 * \brief Prints LaTeX code that can be used as is in LaTeX documents.
 */
class LatexPrintVisitor : public PrintVisitor {
 public:
  LatexPrintVisitor(std::ostream& fs, const std::string& orientation,
                    const std::string& titles, bool colored = true)
      : PrintVisitor(fs, colored), orientation_(orientation), titles_(titles) {
    column_separator_ = " & ";
    line_separator_ = "\\\\\n";
  }

  virtual void visit_PrintMatrix(const PrintMatrix& printMatrix);

  virtual void visit_PrintableTwobitDegree(
      const PrintableTwobitDegree& twoBitDegree);
  virtual void visit_PrintableWord(const PrintableWord& word);

 protected:
  virtual void PrintNumber(uint32_t number, int r, int c, int color, bool box,
                           bool inverse = false);
  virtual void PrintBitcondition(uint8_t bc, int r, int c, int color, bool box,
                                 bool inverse = false);
  virtual void PrintColorCode(uint8_t color, bool inverse = false) const;

  void PrintLatexTableStartDeclaration() const;
  void PrintLatexTableEndDeclaration() const;
  void PrintLatexWordStart() const;
  void PrintLatexWordEnd() const;

  std::string orientation_;
  std::string titles_;
};

#endif  // LATEX_PRINT_VISITOR_H_

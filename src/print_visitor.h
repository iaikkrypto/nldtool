#ifndef PRINT_VISITOR_H_
#define PRINT_VISITOR_H_

#include <cstdint>
#include <iostream>

class PrintMatrix;
class PrintableSpace;
class PrintableString;
class PrintableNumber;
class PrintableTwobitDegree;
class PrintableWord;
class PrintableProbability;
class PrintableProbabilitySum;

/*!
 * \brief Base class for all print visitors.
 *
 * Already implements some sensible default output which can be used by the
 * child classes.
 *
 * All visitors must inherit from the PrintVisitor and can override the
 * provided print output methods. Currently implemented output methods include
 * text and latex output.
 */
class PrintVisitor {
 public:
  enum PColor {
    BLACK,
    RED,
    GREEN,
    BROWN,
    BLUE,
    MAGENTA,
    CYAN,
    LIGHTGRAY,
    DARKGRAY,
    LIGHTRED,
    LIGHTGREEN,
    YELLOW,
    LIGHTBLUE,
    LIGHTMAGENTA,
    LIGHTCYAN,
    WHITE,
    NOCOLOR = 255
  };

  PrintVisitor(std::ostream& fs, bool colored = true)
      : fs_(fs), colored_(colored) {
    column_separator_ = " ";
    line_separator_ = "\n";
  };

  virtual void visit_PrintMatrix(const PrintMatrix& printMatrix);

  // different PrintElements
  virtual void visit_PrintableSpace(const PrintableSpace& space);
  virtual void visit_PrintableString(const PrintableString& string);
  virtual void visit_PrintableNumber(const PrintableNumber& number);
  virtual void visit_PrintableTwobitDegree(
      const PrintableTwobitDegree& twoBitDegree);
  virtual void visit_PrintableWord(const PrintableWord& word);
  virtual void visit_PrintableProbability(
      PrintableProbability&
          probability);  // cant have const due to getProbability()
  virtual void visit_PrintableProbabilitySum(
      PrintableProbabilitySum& probabilitySum);

  virtual ~PrintVisitor(){};

 protected:
  std::ostream& fs_;

  bool colored_;

  std::string column_separator_;
  std::string line_separator_;

  int c_;
  int r_;

  virtual void PrintColorCode(uint8_t color, bool inverse = false) const;
  virtual void PrintNumber(uint32_t number, int r, int c, int color, bool box,
                           bool inverse = false);
  virtual void PrintBitcondition(uint8_t bc, int r, int c, int color, bool box,
                                 bool inverse = false);
  virtual void PrintSmallSeparator() { fs_ << " "; }
  uint8_t BitconditionToColor(uint8_t bc) const;
};

#endif  // PRINT_VISITOR_H_

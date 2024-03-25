#ifndef PRINT_MATRIX_H_
#define PRINT_MATRIX_H_

#include <cstdint>
#include <iostream>
#include <vector>

#include "condition_word.h"
#include "print_visitor.h"
#include "printable.h"

class Characteristic;
class Bitmask;
class Crypto;
class ConditionWord;
typedef std::shared_ptr<ConditionWord> ConditionWordPtr;

typedef unsigned char uint8_t;

/*!
 * \brief A two dimensional matrix containing the printable elements according
 * to the crypto function description.
 */
class PrintMatrix {
  friend class PrintVisitor;
  friend class LatexPrintVisitor;

  /*!
   * \brief Placeholder class for empty matrix cells.
   */
  class PrintableNULL : public Printable {
   public:
    PrintableNULL(bool write_all);
    virtual void Accept(PrintVisitor& printVisitor);
  };

  Printable*** matrix_;
  std::vector<bool> visible_rows_;
  std::vector<bool> visible_columns_;
  std::vector<int> column_widths_;
  int probability_columns_;
  int last_visible_row_;
  int last_visible_column_;
  int rows_;
  int columns_;
  // we have to store separate list of created elements, because elements
  // has not to be inserted into table, or will be inserted multiple times
  std::vector<Printable*> elements_;
  std::string orientation_;
  std::string titles_;
  Printable::PMode printing_mode_;
  bool write_all_;

 public:
  PrintMatrix(bool write_all);

  ~PrintMatrix();

  void SetDimensions(int rows, int columns, int probability_columns);
  void Set(int row, int column, Printable* element);
  void SetHeader(int column, Printable* element);
  Printable* Get(int row, int column);

  void Accept(PrintVisitor& printVisitor);

  void SetLatexColumnOrientation(std::string orientation);
  void SetLatexColumnTitle(std::string titles);
  std::string GetLatexColumnOrientation();
  std::string GetLatexColumnTitle();
  void SetPrintingMode(Printable::PMode printing_mode);
  Printable::PMode GetPrintingMode() { return printing_mode_; }

  PrintableSpace* GetPrintableSpace(int width);
  PrintableString* GetPrintableString(
      std::string value, int printing_width,
      uint8_t color = PrintVisitor::PColor::NOCOLOR);
  PrintableNumber* GetPrintableNumber(int number, int printing_width);
  PrintableTwobitDegree* GetPrintableTwobitDegree(
      ConditionWordPtr word, const Characteristic* characteristic,
      const Characteristic* prev_char);
  PrintableWord* GetPrintableWord(ConditionWordPtr word,
                                  const Characteristic* characteristic,
                                  const Characteristic* prev_char,
                                  Bitmask* condition_word_mask,
                                  Bitmask* prev_condition_word_mask);
  PrintableWordProbability* GetPrintableWordProbability(
      ConditionWordPtr word, const Characteristic* characteristic,
      const Characteristic* prev_char, int space_before = 0,
      int space_after = 0);
  PrintableProbabilitySum* GetPrintableProbabilitySum(
      const Characteristic* characteristic, int space_before = 0,
      int space_after = 0);

 private:
  void Free();
  void Reset();
  void Remove(Printable* element);

  void UpdateVisibility();
  void CalculateColumnWidths();
};

#endif  // PRINT_MATRIX_H_

#include "print_matrix.h"

#include <cassert>
#include <fstream>
#include <iomanip>

#include "bitmask.h"
#include "characteristic.h"
#include "condition_word.h"
#include "crypto.h"
#include "print_visitor.h"

PrintMatrix::PrintMatrix(bool write_all) {
  write_all_ = write_all;
  rows_ = 0;
  columns_ = 0;
  visible_rows_.clear();
  visible_columns_.clear();
  column_widths_.clear();
  last_visible_row_ = 0;
  last_visible_column_ = 0;
  matrix_ = NULL;
  printing_mode_ = Printable::COLORED;
}

PrintMatrix::~PrintMatrix() { Free(); }

void PrintMatrix::SetDimensions(int rows, int columns,
                                int probability_columns) {
  Free();
  probability_columns_ = probability_columns;  // (zero or a number) now I know
                                               // if last_visible_column_
                                               // contains probability columns!
  rows_ = rows + 1;                            // +1 for header
  columns_ = columns;
  visible_rows_.resize(rows_, 0);
  visible_columns_.resize(columns_, 0);
  column_widths_.resize(columns_, 0);
  last_visible_row_ = rows_;
  last_visible_column_ = columns_;
  matrix_ = new Printable**[rows_];
  for (int r = 0; r < rows_; ++r) {
    matrix_[r] = new Printable*[columns_];
    for (int c = 0; c < columns_; ++c) {
      matrix_[r][c] = new PrintableNULL(write_all_);
      elements_.push_back(matrix_[r][c]);
    }
  }
}

void PrintMatrix::Set(int row, int column, Printable* element) {
  assert(rows_ > row && columns_ >= column);
  Remove(matrix_[row + 1][column]);
  matrix_[row + 1][column] = element;
}

void PrintMatrix::SetHeader(int column, Printable* element) {
  assert(columns_ >= column);
  Remove(matrix_[0][column]);
  matrix_[0][column] = element;
}

Printable* PrintMatrix::Get(int row, int column) {
  assert(rows_ > row + 1 && columns_ >= column);
  return matrix_[row + 1][column];
}

void PrintMatrix::Accept(PrintVisitor& printVisitor) {
  Reset();
  UpdateVisibility();
  CalculateColumnWidths();

  printVisitor.visit_PrintMatrix(*this);
}

void PrintMatrix::SetLatexColumnOrientation(std::string orientation) {
  orientation_ = orientation;
}

void PrintMatrix::SetLatexColumnTitle(std::string titles) { titles_ = titles; }

std::string PrintMatrix::GetLatexColumnOrientation() { return orientation_; }

std::string PrintMatrix::GetLatexColumnTitle() { return titles_; }

void PrintMatrix::SetPrintingMode(Printable::PMode printing_mode) {
  printing_mode_ = printing_mode;
}

PrintableSpace* PrintMatrix::GetPrintableSpace(int width) {
  PrintableSpace* tmp = new PrintableSpace(width, write_all_);
  elements_.push_back(tmp);
  return tmp;
}

PrintableString* PrintMatrix::GetPrintableString(std::string value,
                                                 int printing_width,
                                                 uint8_t color) {
  PrintableString* tmp =
      new PrintableString(value, printing_width, color, write_all_);
  elements_.push_back(tmp);
  return tmp;
}

PrintableNumber* PrintMatrix::GetPrintableNumber(int number,
                                                 int printing_width) {
  PrintableNumber* tmp =
      new PrintableNumber(number, printing_width, write_all_);
  elements_.push_back(tmp);
  return tmp;
}

PrintableTwobitDegree* PrintMatrix::GetPrintableTwobitDegree(
    ConditionWordPtr word, const Characteristic* characteristic,
    const Characteristic* prev_char) {
  PrintableTwobitDegree* tmp =
      new PrintableTwobitDegree(word, characteristic, prev_char, write_all_);
  elements_.push_back(tmp);
  return tmp;
}

PrintableWord* PrintMatrix::GetPrintableWord(
    ConditionWordPtr word, const Characteristic* characteristic,
    const Characteristic* prev_char, Bitmask* condition_word_mask,
    Bitmask* prev_condition_word_mask) {
  PrintableWord* tmp =
      new PrintableWord(word, characteristic, prev_char, condition_word_mask,
                        prev_condition_word_mask, write_all_);
  elements_.push_back(tmp);
  return tmp;
}

PrintableWordProbability* PrintMatrix::GetPrintableWordProbability(
    ConditionWordPtr word, const Characteristic* characteristic,
    const Characteristic* prev_char, int space_before, int space_after) {
  PrintableWordProbability* tmp = new PrintableWordProbability(
      word, characteristic, prev_char, space_before, space_after, write_all_);
  elements_.push_back(tmp);
  return tmp;
}

PrintableProbabilitySum* PrintMatrix::GetPrintableProbabilitySum(
    const Characteristic* characteristic, int space_before, int space_after) {
  PrintableProbabilitySum* tmp = new PrintableProbabilitySum(
      characteristic, space_before, space_after, write_all_);
  elements_.push_back(tmp);
  return tmp;
}

void PrintMatrix::Free() {
  for (int i = 0; i < elements_.size(); i++) delete elements_[i];
  if (matrix_) {
    for (int r = 0; r < rows_; ++r) {
      delete[] matrix_[r];
    }
    delete[] matrix_;
  }
  visible_columns_.clear();
  visible_rows_.clear();
  column_widths_.clear();
  matrix_ = NULL;
}

void PrintMatrix::Reset() {
  for (int r = 0; r < last_visible_row_; ++r) {
    for (int c = 0; c < columns_; ++c) {
      matrix_[r][c]->Reset();
    }
  }
}

void PrintMatrix::Remove(Printable* element) {
  for (std::vector<Printable*>::iterator it = elements_.begin();
       it != elements_.end(); ++it)
    if (*it == element) {
      delete *it;
      elements_.erase(it);
      return;
    }
  assert(!"element does not exists");
}

void PrintMatrix::UpdateVisibility() {
  last_visible_row_ = 0;
  last_visible_column_ = 0;

  // search for rows which contain no main elements
  for (int r = 0; r < rows_; ++r) {
    visible_rows_[r] = false;
    for (int c = 0; c < columns_; ++c) {
      if (visible_rows_[r] == true) continue;
      if (matrix_[r][c]->GetLevel() == Printable::VISIBLE) {
        visible_rows_[r] = true;
        last_visible_row_ = r;
      } else if (matrix_[r][c]->GetLevel() == Printable::IGNORABLE)
        for (int r2 = 0; r2 < rows_; ++r2)
          if (matrix_[r2][c]->GetLevel() >= Printable::VISIBLE) {
            visible_rows_[r] = true;
            last_visible_row_ = r;
          }
    }
  }

  // search for columns which contain no main elements
  for (int c = 0; c < columns_; ++c) {
    visible_columns_[c] = false;
    for (int r = 0; r < rows_; ++r) {
      if (visible_columns_[c] == true) continue;
      if (matrix_[r][c]->GetLevel() == Printable::VISIBLE) {
        visible_columns_[c] = true;
        last_visible_column_ = c;
      } else if (matrix_[r][c]->GetLevel() == Printable::IGNORABLE)
        for (int c2 = 0; c2 < columns_; ++c2)
          if (matrix_[r][c2]->GetLevel() == Printable::VISIBLE) {
            visible_columns_[c] = true;
            last_visible_column_ = c;
          }
    }
  }
}

void PrintMatrix::CalculateColumnWidths() {
  for (int c = 0; c < columns_; ++c) {
    column_widths_[c] = 0;
    if (visible_columns_[c] == false) continue;
    for (int r = 0; r < rows_; ++r) {
      if (visible_rows_[r] == false) continue;
      if (column_widths_[c] < matrix_[r][c]->GetWidth())
        column_widths_[c] = matrix_[r][c]->GetWidth();
    }
  }
}

PrintMatrix::PrintableNULL::PrintableNULL(bool write_all)
    : Printable(0, INVISIBLE, write_all) {}

void PrintMatrix::PrintableNULL::Accept(PrintVisitor& printVisitor) {
  // PrintableNULL does not accept any visitors
}

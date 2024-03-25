#include "print_visitor.h"

#include <iomanip>
#include <sstream>

#include "characteristic.h"
#include "print_matrix.h"

void PrintVisitor::visit_PrintMatrix(const PrintMatrix& printMatrix) {
  int y, a, x;
  for (r_ = 0, y = 0; r_ <= printMatrix.last_visible_row_; ++r_) {
    if (printMatrix.visible_rows_[r_] == false) continue;
    for (c_ = 0, a = 0, x = 0; c_ <= printMatrix.last_visible_column_; ++c_) {
      if (printMatrix.visible_columns_[c_] == false) continue;

      // print column separator before element (starting for second element),
      // because we do not know if another element will come
      // and surplus separators are bad in latex tables
      if (a++) {
        fs_ << column_separator_;
        x++;
      }
      Printable* element = printMatrix.matrix_[r_][c_];
      element->Accept(*this);
      x += printMatrix.column_widths_[c_];
      if (element->GetWidth() < printMatrix.column_widths_[c_])
        fs_ << std::setw(printMatrix.column_widths_[c_] - element->GetWidth())
            << " " << std::setw(1);
    }
    y++;
    fs_ << line_separator_;
  }
}

void PrintVisitor::visit_PrintableSpace(const PrintableSpace& space) {
  if (space.width_ > 0) fs_ << std::setw(space.width_) << " " << std::setw(1);
}

void PrintVisitor::visit_PrintableString(const PrintableString& string) {
  int spaces = string.printing_width_ - string.value_.length();

  if (string.write_all_) {
    PrintColorCode(string.color_);
    fs_ << string.value_;
    PrintColorCode(NOCOLOR);
    if (spaces > 0) {
      fs_ << std::setw(spaces) << " " << std::setw(1);
    }
  }
}

void PrintVisitor::visit_PrintableNumber(const PrintableNumber& number) {
  if (number.write_all_)
    fs_ << std::setw(number.width_) << number.number_ << std::setw(1);
}

void PrintVisitor::visit_PrintableTwobitDegree(
    const PrintableTwobitDegree& twoBitDegree) {
  std::vector<uint8_t> twobit_conditions =
      twoBitDegree.characteristic_->GetConstTwobitConditions()
          .GetTwobitDegrees();
  std::vector<uint8_t> prev_twobit_conditions =
      twoBitDegree.prev_char_->GetConstTwobitConditions().GetTwobitDegrees();
  for (int i = twoBitDegree.N_ - 1; i >= 0; --i) {
    const uint8_t val =
        twobit_conditions[(twoBitDegree.word_index_) * twoBitDegree.N_ + i];
    const uint8_t prev_val =
        prev_twobit_conditions[(twoBitDegree.word_index_) * twoBitDegree.N_ +
                               i];
    if (val == 0) {
      PrintSmallSeparator();
      continue;
    }
    if (twoBitDegree.write_all_ || val != prev_val)
      PrintNumber(val, r_, c_ + twoBitDegree.N_ - 1 - i,
                  val < 10 ? LIGHTBLUE : LIGHTMAGENTA, false, false);
  }
}

void PrintVisitor::visit_PrintableWord(const PrintableWord& word) {
#ifdef PRINTHEX
  int val = 0;
  for (int n = word.N_ - 1; n >= 0; --n) {
    uint64_t bc =
        word.word_->GetConditionProxy(n)->GetCondition(*word.characteristic_);
    if (bc == BitCondition("1") || bc == BitCondition("u")) val ^= 1 << n;
  }
  char f = fs_.fill();
  int w = fs_.width();
  // fs << "(" << r << "," << c << ") ";
  fs_ << "0x" << std::setfill('0') << std::setw(word.N_ / 4) << std::hex << val
      << std::dec << std::setfill(f) << std::setw(w);
  return;
#endif

  std::vector<uint8_t> twobit_conditions =
      word.characteristic_->GetConstTwobitConditions().GetTwobitDegrees();

  for (int n = word.N_ - 1; n >= 0; --n) {
    uint64_t bc =
        word.word_->GetConditionProxy(n)->GetCondition(*word.characteristic_);
    uint64_t prev_bc =
        word.word_->GetConditionProxy(n)->GetCondition(*word.prev_char_);

    if (word.word_->GetConditionProxy(n)->GetNumBits() == 1) {
      uint64_t bc =
          word.word_->GetConditionProxy(n)->GetCondition(*word.characteristic_);
      uint64_t prev_bc =
          word.word_->GetConditionProxy(n)->GetCondition(*word.prev_char_);
      std::string bgcol, pbgcol;
      bool box = (twobit_conditions[(word.word_index_) * word.N_ + n] > 0);
      bool inverse = false, prev_inverse = false;
      if (word.condition_word_mask_ && !word.condition_word_mask_->Empty())
        inverse =
            word.condition_word_mask_->GetBit(Bitpos(word.word_index_, n));
      if (word.prev_condition_word_mask_ &&
          !word.prev_condition_word_mask_->Empty())
        prev_inverse =
            word.prev_condition_word_mask_->GetBit(Bitpos(word.word_index_, n));
      if (word.write_all_ || inverse != prev_inverse || bc != prev_bc ||
          bgcol.compare(pbgcol) != 0)
        PrintBitcondition(bc, r_, c_ + word.N_ - 1 - n, BitconditionToColor(bc),
                          box, inverse);
    } else {
      const uint8_t val = word.word_->GetConditionProxy(n)->GetHammingWeight(
          *word.characteristic_);
      const uint8_t prev_val =
          word.word_->GetConditionProxy(n)->GetHammingWeight(*word.prev_char_);
      bool box = (twobit_conditions[(word.word_index_) * word.N_ + n] > 0);
      bool inverse = false, prev_inverse = false;
      if (word.condition_word_mask_ && !word.condition_word_mask_->Empty())
        inverse =
            word.condition_word_mask_->GetBit(Bitpos(word.word_index_, n));
      if (word.prev_condition_word_mask_ &&
          !word.prev_condition_word_mask_->Empty())
        prev_inverse =
            word.prev_condition_word_mask_->GetBit(Bitpos(word.word_index_, n));
      if (word.write_all_ || inverse != prev_inverse || bc != prev_bc ||
          val != prev_val)
        PrintNumber(val, r_, c_ + word.N_ - 1 - n, CYAN, box, inverse);
    }
  }
}

void PrintVisitor::visit_PrintableProbability(
    PrintableProbability& probability) {
  int prec = 3;
  float prob = probability.GetProbability();
  if (std::abs(prob) < 1.0) prec--;
  if (std::abs(prob) < 0.1) prec--;

  if (probability.write_all_) {
    if (probability.space_before_ > 0)
      fs_ << std::setw(probability.space_before_) << " ";
    PrintColorCode(probability.color_);
    fs_ << std::setprecision(prec) << std::setw(5) << prob;
    PrintColorCode(NOCOLOR);
    if (probability.space_after_ > 0)
      fs_ << std::setw(probability.space_after_) << " ";
    fs_ << std::setw(1);
  }
}

void PrintVisitor::visit_PrintableProbabilitySum(
    PrintableProbabilitySum& probabilitySum) {
  PrintColorCode(probabilitySum.color_);
  if (probabilitySum.space_before_ > 0)
    fs_ << std::setw(probabilitySum.space_before_) << " ";
  fs_ << "P:";
  PrintColorCode(NOCOLOR);
  PrintVisitor::visit_PrintableProbability(probabilitySum);
}

void PrintVisitor::PrintNumber(uint32_t number, int /*r*/, int /*c*/, int color,
                               bool /*box*/, bool inverse /* = false*/) {
  PrintColorCode(color, inverse);

  fs_ << (char)(number < 10 ? ('0' + number)
                            : number < 36 ? ('a' + number - 10) : '*');

  PrintColorCode(NOCOLOR);
}

void PrintVisitor::PrintBitcondition(
    uint8_t bc, int /*r*/, int /*c*/, int color, bool /*box*/,
    bool inverse /*= false*/) {  // This method was const!

  static unsigned char chars[] = "#0u3n5x71-ABCDE?";
  assert(static_cast<uint8_t>(bc) < 16);

  PrintColorCode(color, inverse);
  fs_ << chars[bc];

  PrintColorCode(NOCOLOR);
}

void PrintVisitor::PrintColorCode(uint8_t color,
                                  bool inverse /*= false*/) const {
  if (colored_) {
    if (color == NOCOLOR && !inverse) {
      fs_ << "\033[0m";
      return;
    }
    fs_ << "\033[" << color / 8 << ";3" << color % 8 << "m";
    if (inverse) fs_ << "\033[7m";
  }
}

uint8_t PrintVisitor::BitconditionToColor(uint8_t bc) const {
  assert(static_cast<uint8_t>(bc) < 16);
  static int color[] = {LIGHTMAGENTA, NOCOLOR, LIGHTRED,   YELLOW,
                        LIGHTRED,     YELLOW,  LIGHTGREEN, YELLOW,
                        NOCOLOR,      NOCOLOR, YELLOW,     YELLOW,
                        YELLOW,       YELLOW,  YELLOW,     YELLOW};
  return color[bc];
}

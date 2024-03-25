#include "printable.h"

#include <cassert>
#include <fstream>
#include <iomanip>

#include "bitmask.h"
#include "characteristic.h"
#include "condition_word.h"
#include "crypto.h"
#include "print_visitor.h"

Printable::Printable(int printing_width, PLevel level, bool write_all) {
  write_all_ = write_all;
  level_ = level;
  printing_width_ = printing_width;
}

Printable::~Printable() {}

Printable::PLevel Printable::GetLevel() const { return level_; }

Printable* Printable::SetLevel(PLevel level) {
  level_ = level;
  return this;
}

void Printable::Reset() {}

int Printable::GetWidth() { return printing_width_; }

PrintableSpace::PrintableSpace(int width, bool write_all)
    : Printable(width, IGNORABLE, write_all) {
  width_ = width;
}

PrintableSpace::~PrintableSpace() {}

void PrintableSpace::Accept(PrintVisitor& printVisitor) {
  printVisitor.visit_PrintableSpace(*this);
}

PrintableString::PrintableString(std::string value, int printing_width,
                                 uint8_t color, bool write_all)
    : Printable(std::max((int)value.length(), printing_width), IGNORABLE,
                write_all) {
  value_ = value;
  color_ = color;
}

PrintableString::~PrintableString() {}

void PrintableString::Accept(PrintVisitor& printVisitor) {
  printVisitor.visit_PrintableString(*this);
}

PrintableNumber::PrintableNumber(int number, int printing_width, bool write_all)
    : Printable(printing_width, IGNORABLE, write_all) {
  number_ = number;
  width_ = printing_width;
}

PrintableNumber::~PrintableNumber() {}

void PrintableNumber::Accept(PrintVisitor& printVisitor) {
  printVisitor.visit_PrintableNumber(*this);
}

PrintableTwobitDegree::PrintableTwobitDegree(
    ConditionWordPtr word, const Characteristic* characteristic,
    const Characteristic* prev_char, bool write_all)
    : Printable(characteristic->GetCrypto()->GetWordSize(), VISIBLE,
                write_all) {
  assert(word);
  word_ = word;
  characteristic_ = characteristic;
  prev_char_ = prev_char;
  N_ = characteristic_->GetCrypto()->GetWordSize();
  word_index_ = word->GetConditionMaskIndex();
}

PrintableTwobitDegree::~PrintableTwobitDegree() {}

void PrintableTwobitDegree::Accept(PrintVisitor& printVisitor) {
  printVisitor.visit_PrintableTwobitDegree(*this);
}

PrintableWord::PrintableWord(ConditionWordPtr word,
                             const Characteristic* characteristic,
                             const Characteristic* prev_char,
                             Bitmask* condition_word_mask,
                             Bitmask* prev_condition_word_mask, bool write_all)
    : Printable(characteristic->GetCrypto()->GetWordSize(), VISIBLE,
                write_all) {
  assert(word && characteristic);
  word_ = word;
  characteristic_ = characteristic;
  prev_char_ = prev_char;
  prev_condition_word_mask_ = prev_condition_word_mask;
  condition_word_mask_ = condition_word_mask;
  N_ = characteristic_->GetCrypto()->GetWordSize();
  word_index_ = word_->GetConditionMaskIndex();
}

PrintableWord::~PrintableWord() {}

void PrintableWord::Accept(PrintVisitor& printVisitor) {
  printVisitor.visit_PrintableWord(*this);
}

PrintableProbability::PrintableProbability(int space_before, int space_after,
                                           uint8_t color, bool write_all)
    : Printable(space_before + 5 + space_after, IGNORABLE, write_all) {
  space_before_ = space_before;
  space_after_ = space_after;
  color_ = color;
}

void PrintableProbability::Accept(PrintVisitor& printVisitor) {
  printVisitor.visit_PrintableProbability(*this);
}

PrintableWordProbability::PrintableWordProbability(
    ConditionWordPtr word, const Characteristic* characteristic,
    const Characteristic* prev_char, int space_before, int space_after,
    bool write_all)
    : PrintableProbability(space_before, space_after,
                           PrintVisitor::PColor::WHITE, write_all) {
  assert(word && characteristic);
  word_ = word;
  characteristic_ = characteristic;
  prev_char_ = prev_char;
  N_ = characteristic->GetCrypto()->GetWordSize();
  word_index_ = word_->GetConditionMaskIndex();
  calculated_ = false;
  probability_ = 0;
}

PrintableWordProbability::~PrintableWordProbability() {}

float PrintableWordProbability::GetProbability() {
  if (calculated_) return probability_;
  probability_ = characteristic_->GetProbability(word_index_);
  calculated_ = true;
  //  probability_ = 0;
  //  for (int n = N_ - 1; n >= 0; --n) {
  //    uint8_t bc = characteristic_->GetBitCondition(Bitpos(word_index_,
  //    n)).GetValue(); switch (bc) {
  //      case 1:  // 0
  //      case 2:  // u
  //      case 4:  // n
  //      case 8:  // 1
  //        probability_ -= 1.0;
  //        break;
  //    }
  //  }
  return probability_;
}

PrintableProbabilitySum::PrintableProbabilitySum(
    const Characteristic* characteristic, int space_before, int space_after,
    bool write_all)
    : PrintableProbability(space_before, space_after, PrintVisitor::PColor::RED,
                           write_all),
      characteristic_(characteristic) {}

void PrintableProbabilitySum::Accept(PrintVisitor& printVisitor) {
  printVisitor.visit_PrintableProbabilitySum(*this);
}

float PrintableProbabilitySum::GetProbability() {
  return characteristic_->GetProbabilitySum();
}

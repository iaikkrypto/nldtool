#ifndef TWOBIT_CONDITION_H_
#define TWOBIT_CONDITION_H_

#include "condition_proxy.h"

/*!
 * \brief Class representing a twobit condition according to Leurent.
 *
 * For a detailed explanation of twobit conditions see:
 *   Gaëtan Leurent: Analysis of Differential Attacks in ARX Constructions.
 *   ASIACRYPT 2012 http://dx.doi.org/10.1007/978-3-642-34961-4_15
 */
class TwobitCondition {
 public:
  TwobitCondition() : first_(0, 0), second_(0, 0), condition_(0) {}

  virtual ~TwobitCondition() {}

  TwobitCondition(const TwobitCondition& s)
      : first_(s.first_), second_(s.second_), condition_(s.condition_) {}

  TwobitCondition(Bitpos first, Bitpos second, uint8_t value)
      : first_(first), second_(second), condition_(value) {}

  TwobitCondition& operator=(const TwobitCondition& rhs) {
    first_ = rhs.first_;
    second_ = rhs.second_;
    condition_ = rhs.condition_;
    return *this;
  }

  bool operator<(const TwobitCondition& rhs) const {
    if (first_ < rhs.first_)
      return true;
    else if (rhs.first_ < first_)
      return false;
    else if (second_ < rhs.second_)
      return true;
    else if (rhs.second_ < second_)
      return false;
    else if (condition_ < rhs.condition_)
      return true;
    else
      return false;
  }

  friend std::ostream& operator<<(std::ostream& os, const TwobitCondition& t) {
    os << t.first_;
    //    os << (t.condition_ ? "==" : "!=");
    assert(t.condition_ < 16);
    if (t.condition_ == 0)
      os << "#=";
    else if (t.condition_ == 1)
      os << "==";
    else if (t.condition_ == 2)
      os << "!=";
    else if (t.condition_ == 3)
      os << "?=";
    else
      os << std::hex << (int)t.condition_ << std::dec << "=";
    os << t.second_;
    return os;
  }

  Bitpos first_;
  Bitpos second_;
  uint8_t condition_;
};

#endif  // TWOBIT_CONDITION_H_

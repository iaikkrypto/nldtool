#ifndef CONDITION_H_
#define CONDITION_H_

#include <cassert>
#include <cstdint>
#include <iostream>

#include "utils.h"

/*!
 * \brief General description of a Condition, some specializations are
 * implemented explicitly.
 */
template <int N>
class Condition {
 public:
  static const int SIZE = ((1 << N) * (1 << N)) / 64;
  Condition() {
    for (int i = 0; i < SIZE; ++i) condition_[i] = ~0ull;
  }
  Condition(uint64_t condition[SIZE]) : condition_(condition) {}
  bool operator==(const Condition<N>& cc) const {
    bool result = true;
    for (int i = 0; i < SIZE; ++i) result &= condition_[i] == cc.condition_[i];
    return result;
  }
  bool operator!=(const Condition<N>& cc) const { return !(*this == cc); }
  void SetCondition(uint64_t condition[SIZE]) {
    for (int i = 0; i < SIZE; ++i) condition_[i] = condition[i];
  }
  void GetCondition(uint64_t condition[SIZE]) const {
    for (int i = 0; i < SIZE; ++i) condition[i] = condition_[i];
  }

 private:
  uint64_t condition_[SIZE];
};

/*!
 * \brief Describes the conditions on one bit of a differential characteristic.
 *
 * See Chapter 3.2 "Generalized Conditions" in
 *   Christophe De Cannière, Christian Rechberger: Finding SHA-1
 * Characteristics: General Results and Applications. ASIACRYPT 2006
 * http://dx.doi.org/10.1007/11935230_1
 */
template <>
class Condition<1> {
 public:
  Condition<1>() : condition_(0xf) {}
  explicit Condition<1>(uint8_t condition) : condition_(condition & 0xf) {
    assert((condition & 0xf) == condition);
  }
  explicit Condition<1>(const char condition[2])
      : condition_(CHAR2COND(condition[0])) {
    assert((condition_ & 0xf) == condition_);
  }
  bool operator<(const Condition<1> bc) const {
    return condition_ < bc.condition_;
  }
  bool operator==(const Condition<1> bc) const {
    return condition_ == bc.condition_;
  }
  bool operator!=(const Condition<1> bc) const {
    return condition_ != bc.condition_;
  }
  bool Set(uint8_t condition) {
    condition_ = condition & 0xf;
    assert((condition & 0xf) == condition);
    return (condition & 0xf) == condition;
  }
  operator uint8_t() const { return condition_; }
  char GetChar() const { return COND2CHAR(condition_); }
  bool Set(std::string s) { return Set(CHAR2COND(s[0])); }
  uint8_t GetPair(int index) const { return PAIRS[condition_][index]; }
  int GetNumPairs() const { return NUMPAIRS[condition_]; }
  Condition<1>& Filter(Condition<1> bc) {
    condition_ &= bc.condition_;
    return *this;
  }

  static const int NUMPAIRS[16];
  static const int PAIRS[16][4];
  static const int PAIRS4[16][4];

  static const class Cond2Char {
   public:
    Cond2Char();
    char operator()(uint8_t bc) const {
      assert(static_cast<uint8_t>(bc) < 16);
      return cond2char[bc];
    }

   private:
    char cond2char[16];
  } COND2CHAR;

  static const class Char2Cond {
   public:
    Char2Cond();
    uint8_t operator()(char c) const {
      assert(char2cond[static_cast<uint8_t>(c)] != 255);
      return char2cond[static_cast<uint8_t>(c)];
    }

   private:
    uint8_t char2cond[256];
  } CHAR2COND;

 private:
  uint8_t condition_;
};

/*!
 * \brief Combination of two \see Condition<1>
 */
template <>
class Condition<2> {
 public:
  Condition<2>() : condition_(0xffff) {}
  Condition<2>(uint16_t condition) : condition_(condition & 0xffff) {
    assert((condition & 0xffff) == condition);
  }
  bool operator==(const Condition<2> cc) const {
    return condition_ == cc.condition_;
  }
  bool operator!=(const Condition<2> cc) const {
    return condition_ != cc.condition_;
  }
  bool Set(uint16_t condition) {
    condition_ = condition & 0xffff;
    assert((condition & 0xffff) == condition);
    return (condition & 0xffff) == condition;
  }
  operator uint16_t() const { return condition_; }
  Condition<2>& Filter0(Condition<1> bc) {
    uint16_t tmp = 0;
    for (int i = 0; i < 4; ++i) tmp |= uint16_t(bc) << (i * 4);
    condition_ &= tmp;
    assert((condition_ & 0xffff) == condition_);
    return *this;
  }
  Condition<2>& Filter1(Condition<1> bc) {
    uint16_t tmp = 0;
    for (int i = 0; i < 4; ++i) {
      const bool t = bc & (1 << i);
      tmp |= (0x000f << (i * 4)) & (((uint16_t)!t) - 1);
    }
    condition_ &= tmp;
    assert((condition_ & 0xffff) == condition_);
    return *this;
  }
  Condition<2>& Filter(Condition<2> bc) {
    condition_ &= bc.condition_;
    assert((condition_ & 0xffff) == condition_);
    return *this;
  }

  Condition<2>(Condition<1> bit1, Condition<1> bit0) {
    // TODO: Change
    const int onebit_to_twobit_cond[4][4] = {
        {0, 1, 4, 5}, {2, 3, 6, 7}, {8, 9, 12, 13}, {10, 11, 14, 15}};
    condition_ = 0;
    for (int i = 0; i < 4; i++)
      if (((bit1 >> i) & 1) == 1)
        for (int j = 0; j < 4; j++)
          if (((bit0 >> j) & 1) == 1)
            condition_ |= 1 << onebit_to_twobit_cond[i][j];
  }

  void MergeBit0(Condition<1> cond);
  void MergeBit1(Condition<1> cond);
  Condition<1> GetBit0();
  Condition<1> GetBit1();

 private:
  uint16_t condition_;
  // Two Bit Conditions per Bit set, from high to low.
  // 11,1n,n1,nn,1u,10,nu,n0,u1,un,01,0n,uu,u0,0u,00
};
/*!
 * \brief Combination of three \see Condition<1>
 */
template <>
class Condition<3> {
 public:
  Condition<3>() : condition_(~0ull) {}
  Condition<3>(uint64_t condition) : condition_(condition) {}
  bool operator==(const Condition<3> cc) const {
    return condition_ == cc.condition_;
  }
  bool operator!=(const Condition<3> cc) const {
    return condition_ != cc.condition_;
  }
  void Set(uint64_t condition) { condition_ = condition; }
  operator uint64_t() const { return condition_; }
  Condition<3>& Filter0(Condition<1> bc) {
    uint64_t tmp = 0;
    for (int i = 0; i < 16; ++i) tmp |= uint64_t(bc) << (i * 4);
    condition_ &= tmp;
    return *this;
  }
  Condition<3>& Filter1(Condition<1> bc) {
    uint64_t tmp = 0;
    for (int i = 0; i < 4; ++i) {
      const bool t = bc & (1 << i);
      tmp |= (0x000f000f000f000full << (i * 4)) & (((uint64_t)!t) - 1);
    }
    condition_ &= tmp;
    return *this;
  }
  Condition<3>& Filter2(Condition<1> bc) {
    uint64_t tmp = 0;
    for (int i = 0; i < 4; ++i) {
      const bool t = bc & (1 << i);
      tmp |= (0x000000000000ffffull << (i * 16)) & (((uint64_t)!t) - 1);
    }
    condition_ &= tmp;
    return *this;
  }
  Condition<3>& Filter(Condition<3> bc) {
    condition_ &= bc.condition_;
    return *this;
  }

  void MergeBit0(Condition<1> cond);
  void MergeBit1(Condition<1> cond);
  void MergeBit2(Condition<1> cond);
  void MergeBits01(Condition<2> cond);
  void MergeBits12(Condition<2> cond);

  Condition<1> GetBit0();
  Condition<1> GetBit1();
  Condition<1> GetBit2();
  Condition<2> GetBits01();
  Condition<2> GetBits12();

 private:
  uint64_t condition_;
};

typedef Condition<1> BitCondition;

#endif  // CONDITION_H_

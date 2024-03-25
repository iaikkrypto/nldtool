#ifndef BITPOS_H_
#define BITPOS_H_

#include <cassert>
#include <cstdint>
#include <ostream>

/*!
 * \brief Identifies a bit by specifying the word it is contained in and its
 * position in that word.
 */
class Bitpos {
 public:
  Bitpos() : word_(0), bit_(0) {}

  Bitpos(int step, int bit) : word_(step), bit_(bit) {}

  Bitpos(const Bitpos& b) : word_(b.word_), bit_(b.bit_) {}

  Bitpos& operator=(const Bitpos& rhs) {
    word_ = rhs.word_;
    bit_ = rhs.bit_;
    return *this;
  }

  bool operator==(const Bitpos& rhs) const {
    return word_ == rhs.word_ && bit_ == rhs.bit_;
  }

  bool operator<(const Bitpos& rhs) const {
    assert(0 <= word_ && word_ < MAXWORDSIZE);
    assert(0 <= bit_ && bit_ < MAXBITSIZE);
    assert(0 <= rhs.word_ && rhs.word_ < MAXWORDSIZE);
    assert(0 <= rhs.bit_ && rhs.bit_ < MAXBITSIZE);
    return ((((uint64_t)word_) << 32) | (bit_)) <
           ((((uint64_t)rhs.word_) << 32) | (rhs.bit_));
  }

  int GetWord() const { return word_; }

  void SetWord(int word) { word_ = word; }

  int GetBit() const { return bit_; }

  void SetBit(int bit) { bit_ = bit; }

  friend std::ostream& operator<<(std::ostream& os, const Bitpos& s) {
    os << "(" << s.GetWord() << "," << s.GetBit() << ")";
    return os;
  }

 protected:
  int word_;
  int bit_;
  static const int MAXWORDSIZE = 1 << 30;
  static const int MAXBITSIZE = 1 << 30;
};

#endif  // BITPOS_H_

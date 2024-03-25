#ifndef BITSLICE_TWOBIT_CONDITION_H_
#define BITSLICE_TWOBIT_CONDITION_H_

#include <cassert>
#include <cstdint>
#include <ostream>

/*!
 * \brief Small representation of a twobit condition for \see
 * PropagateTwobitOutput.
 */
class BitsliceTwobitCondition {
 public:
  uint8_t a;
  uint8_t b;
  uint8_t c;

  friend std::ostream& operator<<(std::ostream& os,
                                  BitsliceTwobitCondition& t) {
    assert(t.c < 16);
    os << (int)t.a;
    if (t.c == 0)
      os << "#=";
    else if (t.c == 1)
      os << "==";
    else if (t.c == 2)
      os << "!=";
    else if (t.c == 3)
      os << "?=";
    else
      os << ((char)t.c + '0') << "=";
    os << (int)t.b;
    return os;
  }
};

#endif  // BITSLICE_TWOBIT_CONDITION_H_

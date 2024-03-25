#ifndef PAIR_H_
#define PAIR_H_

#include <cstdint>

/*!
 * \brief Custom Pair class used for describing 2 bits.
 *
 * All possible mathematical relations are overloaded to enable easy
 * computation with pairs (for example, in functions (\see IF, \see MAJ, etc.)).
 */
class Pair {
 public:
  uint8_t first;
  uint8_t second;

  Pair& operator+=(Pair rhs);
  Pair& operator-=(Pair rhs);
  Pair& operator*=(Pair rhs);
  Pair& operator/=(Pair rhs);
  Pair& operator%=(Pair rhs);
  Pair& operator&=(Pair rhs);
  Pair& operator|=(Pair rhs);
  Pair& operator^=(Pair rhs);
  Pair& operator>>=(int i);
  Pair& operator<<=(int i);
  Pair& operator++();
  Pair& operator--();
  Pair operator++(int);
  Pair operator--(int);

  Pair& operator+=(uint8_t value);
  Pair& operator-=(uint8_t value);
  Pair& operator*=(uint8_t value);
  Pair& operator/=(uint8_t value);
  Pair& operator%=(uint8_t value);
  Pair& operator&=(uint8_t value);
  Pair& operator|=(uint8_t value);
  Pair& operator^=(uint8_t value);

  friend Pair operator~(Pair rhs);
  friend Pair operator!(Pair rhs);
  friend bool operator!=(Pair lhs, Pair rhs);
  friend bool operator==(Pair lhs, Pair rhs);
  friend Pair operator+(Pair lhs, Pair rhs);
  friend Pair operator-(Pair lhs, Pair rhs);
  friend Pair operator*(Pair lhs, Pair rhs);
  friend Pair operator/(Pair lhs, Pair rhs);
  friend Pair operator%(Pair lhs, Pair rhs);
  friend Pair operator&(Pair lhs, Pair rhs);
  friend Pair operator|(Pair lhs, Pair rhs);
  friend Pair operator^(Pair lhs, Pair rhs);
  friend Pair operator>>(Pair lhs, int i);
  friend Pair operator<<(Pair lhs, int i);

  friend Pair operator+(Pair lhs, uint8_t value);
  friend Pair operator-(Pair lhs, uint8_t value);
  friend Pair operator*(Pair lhs, uint8_t value);
  friend Pair operator/(Pair lhs, uint8_t value);
  friend Pair operator%(Pair lhs, uint8_t value);
  friend Pair operator&(Pair lhs, uint8_t value);
  friend Pair operator|(Pair lhs, uint8_t value);
  friend Pair operator^(Pair lhs, uint8_t value);
};

inline Pair& Pair::operator+=(Pair rhs) {
  first += rhs.first;
  second += rhs.second;
  return *this;
}

inline Pair& Pair::operator-=(Pair rhs) {
  first -= rhs.first;
  second -= rhs.second;
  return *this;
}

inline Pair& Pair::operator*=(Pair rhs) {
  first *= rhs.first;
  second *= rhs.second;
  return *this;
}

inline Pair& Pair::operator/=(Pair rhs) {
  first /= rhs.first;
  second /= rhs.second;
  return *this;
}

inline Pair& Pair::operator%=(Pair rhs) {
  first %= rhs.first;
  second %= rhs.second;
  return *this;
}

inline Pair& Pair::operator&=(Pair rhs) {
  first &= rhs.first;
  second &= rhs.second;
  return *this;
}

inline Pair& Pair::operator|=(Pair rhs) {
  first |= rhs.first;
  second |= rhs.second;
  return *this;
}

inline Pair& Pair::operator^=(Pair rhs) {
  first ^= rhs.first;
  second ^= rhs.second;
  return *this;
}

inline Pair& Pair::operator>>=(int i) {
  first >>= i;
  second >>= i;
  return *this;
}

inline Pair& Pair::operator<<=(int i) {
  first <<= i;
  second <<= i;
  return *this;
}

inline Pair& Pair::operator++() {
  ++first;
  ++second;
  return *this;
}

inline Pair& Pair::operator--() {
  --first;
  --second;
  return *this;
}

inline Pair Pair::operator++(int) {
  Pair y(*this);
  operator++();
  return y;
}

inline Pair Pair::operator--(int) {
  Pair y(*this);
  operator--();
  return y;
}

inline Pair& Pair::operator+=(uint8_t value) {
  first += value;
  second += value;
  return *this;
}

inline Pair& Pair::operator-=(uint8_t value) {
  first -= value;
  second -= value;
  return *this;
}

inline Pair& Pair::operator*=(uint8_t value) {
  first *= value;
  second *= value;
  return *this;
}

inline Pair& Pair::operator/=(uint8_t value) {
  first /= value;
  second /= value;
  return *this;
}

inline Pair& Pair::operator%=(uint8_t value) {
  first %= value;
  second %= value;
  return *this;
}

inline Pair& Pair::operator&=(uint8_t value) {
  first &= value;
  second &= value;
  return *this;
}

inline Pair& Pair::operator|=(uint8_t value) {
  first |= value;
  second |= value;
  return *this;
}

inline Pair& Pair::operator^=(uint8_t value) {
  first ^= value;
  second ^= value;
  return *this;
}

inline Pair operator~(Pair rhs) {
  rhs.first = ~rhs.first;
  rhs.second = ~rhs.second;
  return rhs;
}

inline Pair operator!(Pair rhs) {
  rhs.first = !rhs.first;
  rhs.second = !rhs.second;
  return rhs;
}

inline bool operator==(Pair lhs, Pair rhs) {
  return (lhs.first == rhs.first) && (lhs.second == rhs.second);
}

inline bool operator!=(Pair lhs, Pair rhs) {
  return (lhs.first != rhs.first) || (lhs.second != rhs.second);
}

inline Pair operator+(Pair lhs, Pair rhs) { return lhs += rhs; }

inline Pair operator-(Pair lhs, Pair rhs) { return lhs -= rhs; }

inline Pair operator*(Pair lhs, Pair rhs) { return lhs *= rhs; }

inline Pair operator/(Pair lhs, Pair rhs) { return lhs /= rhs; }

inline Pair operator%(Pair lhs, Pair rhs) { return lhs %= rhs; }

inline Pair operator&(Pair lhs, Pair rhs) { return lhs &= rhs; }

inline Pair operator|(Pair lhs, Pair rhs) { return lhs |= rhs; }

inline Pair operator^(Pair lhs, Pair rhs) { return lhs ^= rhs; }

inline Pair operator>>(Pair lhs, int i) { return lhs >>= i; }

inline Pair operator<<(Pair lhs, int i) { return lhs <<= i; }

inline Pair operator+(Pair lhs, uint8_t value) { return lhs += value; }

inline Pair operator-(Pair lhs, uint8_t value) { return lhs -= value; }

inline Pair operator*(Pair lhs, uint8_t value) { return lhs *= value; }

inline Pair operator/(Pair lhs, uint8_t value) { return lhs /= value; }

inline Pair operator%(Pair lhs, uint8_t value) { return lhs %= value; }

inline Pair operator&(Pair lhs, uint8_t value) { return lhs &= value; }

inline Pair operator|(Pair lhs, uint8_t value) { return lhs |= value; }

inline Pair operator^(Pair lhs, uint8_t value) { return lhs ^= value; }

#endif  // PAIR_H_

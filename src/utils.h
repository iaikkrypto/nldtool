#ifndef UTILS_H_
#define UTILS_H_

#include <cassert>
#include <cstdint>

namespace nldtool {

template <class T>
inline int HammingWeight(T v) {
  v = v - ((v >> 1) & (T) ~(T)0 / 3);
  v = (v & (T) ~(T)0 / 15 * 3) + ((v >> 2) & (T) ~(T)0 / 15 * 3);
  v = (v + (v >> 4)) & (T) ~(T)0 / 255 * 15;
  return (T)(v * ((T) ~(T)0 / 255)) >> (sizeof(T) - 1) * 8;
}

template <class T>
inline T LeastSignificantSetBitmask(T x) {
  return x & ~(x - 1);
}

inline uint64_t Mask(int n) {
  assert(n >= 0 && n <= 64);
  return (~static_cast<uint64_t>(0)) >> (64 - n);
}

inline void SetBits(uint64_t& word, uint64_t mask, bool b) {
  word = (word & ~mask) | (-b & mask);
}

inline void SetBit(uint64_t& word, int n, bool b) {
  assert(n >= 0 && n < 64);
  SetBits(word, static_cast<uint64_t>(1) << n, b);
}

inline void SetBit(uint64_t& word, int n) {
  assert(n >= 0 && n < 64);
  word |= static_cast<uint64_t>(1) << n;
}

inline void ClrBit(uint64_t& word, int n) {
  assert(n >= 0 && n < 64);
  word &= ~(static_cast<uint64_t>(1) << n);
}

inline uint64_t GetBit(uint64_t word, int n) {
  assert(n >= 0 && n < 64);
  return (word >> n) & 1;
}

inline void SetWindowBits(uint64_t& word, int n, int w, uint64_t b) {
  assert(n >= 0 && n < 64);
  assert(w >= 1 && w <= 64 - n);
  const uint64_t bits = b << n;
  const uint64_t mask = Mask(w) << n;
  word = (word & ~mask) | (bits & mask);
}

inline uint64_t GetWindowBits(uint64_t word, int n, int w) {
  assert(n >= 0 && n < 64);
  assert(w >= 1 && w <= 64 - n);
  return (word >> n) & Mask(w);
}

inline void SwapBits(uint64_t& a, int i, int j, int w) {
  const uint64_t& ai = nldtool::GetWindowBits(a, i * w, w);
  const uint64_t& aj = nldtool::GetWindowBits(a, j * w, w);
  nldtool::SetWindowBits(a, i * w, w, aj);
  nldtool::SetWindowBits(a, j * w, w, ai);
}

inline uint64_t Shr(uint64_t w, int x, int N) {
  assert(0 <= x && x < N);
  return (w >> x) & nldtool::Mask(N - x);
}

inline uint64_t Shl(uint64_t w, int x, int N) {
  assert(0 <= x && x < N);
  return (w << x) & nldtool::Mask(N);
}

inline uint64_t Rotr(uint64_t w, int x, int N) {
  assert(0 <= x && x < N);
  return (((w & nldtool::Mask(N)) >> x) | (w << (N - x))) & nldtool::Mask(N);
}

inline uint64_t Rotl(uint64_t w, int x, int N) {
  assert(0 <= x && x < N);
  return ((w << x) | ((w & nldtool::Mask(N)) >> (N - x))) & nldtool::Mask(N);
}

template <int N>
inline uint64_t Shr(uint64_t w, int x) {
  assert(0 <= x && x < N);
  return (w >> x) & nldtool::Mask(N - x);
}

template <int N>
inline uint64_t Shl(uint64_t w, int x) {
  assert(0 <= x && x < N);
  return (w << x) & nldtool::Mask(N);
}

template <int N>
inline uint64_t Rotr(uint64_t w, int x) {
  assert(0 <= x && x < N);
  return (((w & nldtool::Mask(N)) >> x) | (w << (N - x))) & nldtool::Mask(N);
}

template <int N>
inline uint64_t Rotl(uint64_t w, int x) {
  assert(0 <= x && x < N);
  return (((w & nldtool::Mask(N)) << x) | (w >> (N - x))) & nldtool::Mask(N);
}

constexpr int Log2(int x) { return (x < 2) ? 0 : 1 + Log2(x >> 1); }

}  // namespace nldtool

#endif  // UTILS_H_

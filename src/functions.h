#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

#include <cmath>
#include <cstdint>
#include <functional>
#include <numeric>
#include <string>

#include "pair.h"
#include "utils.h"

/*!
 * \brief Base class for functions used in steps of crypto functions.
 *
 * All methods and members of the class have to be static and members have to
 * be const or constexpr.
 */
class F {
 public:
  static constexpr char kName[] = "F";
  static const int kNumInputs = 3;
  static const int kNumOutputs = 1;
  static const int kPrevState = 0;
  static const int kNextState = 0;
  static const int kStateSize = 1;
  static constexpr int Symmetry(int i) { return i; }
  static constexpr int Bitsize(int i) { return 1; }
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {}
  template <class T>
  static void f(int word_size, const T x[kNumInputs], T y[kNumOutputs]) {}
};

/*!
 * \brief The ONX function for 3 inputs, as used in MD5 and RIPEMD.
 */
class ONX : public F {
 public:
  static constexpr char kName[] = "ONX";
  static const int kNumInputs = 3;
  static const int kNumOutputs = 1;
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    y[0] = x[1] ^ (x[0] | (!x[2]));
  }
};

/*!
 * \brief The IF function for 3 inputs, as used in MD4/5, SHA-1/2 etc.
 */
class IF : public F {
 public:
  static constexpr char kName[] = "IF";
  static const int kNumInputs = 3;
  static const int kNumOutputs = 1;
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    y[0] = (x[0] & x[1]) | ((!x[0]) & x[2]);
  }
};

/*!
 * \brief The MAJ function for 3 inputs, as used in MD4, SHA-1/2 etc.
 */
class MAJ : public F {
 public:
  static constexpr char kName[] = "MAJ";
  static const int kNumInputs = 3;
  static const int kNumOutputs = 1;
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    y[0] = (x[0] & x[1]) ^ (x[1] & x[2]) ^ (x[2] & x[0]);
  }
};

/*!
 * \brief Template class that describes an S-Box with a lookup table.
 */
template <int I, int O, const uint8_t LUT[]>
class SBOX : public F {
 public:
  static_assert(I > 0 && I <= 8, "The maximum number of inputs is 8");
  static_assert(O > 0 && O <= 8, "The maximum number of outputs is 8");
  static constexpr char kName[] = "SBOX<I,O,LUT>";
  static const int kNumInputs = I;
  static const int kNumOutputs = O;
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    uint64_t input = 0, output;
    for (int i = 0; i < kNumInputs; i++) {
      input <<= 1;
      input |= (x[i] & 1);
    };
    output = LUT[input];
    for (int i = 0; i < kNumOutputs; i++) {
      y[kNumOutputs - 1 - i] = output & 1;
      output >>= 1;
    }
  }
  static void f(const Pair x[kNumOutputs], Pair y[kNumOutputs]) {
    uint64_t input1 = 0, output1, input2 = 0, output2;
    for (int i = 0; i < kNumInputs; i++) {
      input1 <<= 1;
      input1 |= (x[i].first & 1);
      input2 <<= 1;
      input2 |= (x[i].second & 1);
    };
    output1 = LUT[input1];
    output2 = LUT[input2];
    for (int i = 0; i < kNumOutputs; i++) {
      y[kNumOutputs - 1 - i].first = output1 & 1;
      output1 >>= 1;
      y[kNumOutputs - 1 - i].second = output2 & 1;
      output2 >>= 1;
    }
  }
};

template <int I, int O, const uint8_t LUT[]>
constexpr char SBOX<I, O, LUT>::kName[];

/*!
 * \brief Template class implementing an XOR with N inputs.
 */
template <int N>
class XOR : public F {
 public:
  static constexpr char kName[] = "XOR<N>";
  static const int kNumInputs = N;
  static const int kNumOutputs = 1;
  static constexpr int Symmetry(int i) { return i == N; }
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    y[0] = std::accumulate(&x[1], &x[kNumInputs], x[0], std::bit_xor<T>());
  }
  template <class T>
  static void f(int word_size, const T x[kNumInputs], T y[kNumOutputs]) {
    return f(x, y);
  }
};

template <int N>
constexpr char XOR<N>::kName[];

/*!
 * \brief Template class implementing an XOR with N inputs and 2-bit
 * constraints.
 *
 * Like XOR, but takes twobit conditions into account. For a detailed
 * explanation see Gaëtan Leurent: Analysis of Differential Attacks in ARX
 * Constructions. ASIACRYPT 2012 http://dx.doi.org/10.1007/978-3-642-34961-4_15
 */
template <int N>
class XORB : public F {
 public:
  static constexpr char kName[] = "XORB<N>";
  static const int kNumInputs = N;
  static const int kNumOutputs = 1;
  static constexpr int Symmetry(int i) { return i == N; }
  static constexpr int Bitsize(int i) { return 2; }
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    y[0] = std::accumulate(&x[1], &x[kNumInputs], x[0], std::bit_xor<T>());
  }
};

template <int N>
constexpr char XORB<N>::kName[];

/*!
 * \brief Template class implementing a modular addition with N inputs.
 *
 * In addition to the normal in- and outputs, there is a carry input and a
 * carry output of the required size to hold the carry information. One can use
 * \see CarryStep to automatically insert the required carry words.
 */
template <int N>
class ADD : public F {
 public:
  static constexpr char kName[] = "ADD<N>";
  static const int kNumInputs = N + 1;
  static const int kNumOutputs = 2;
  static const int kPrevState = kNumInputs - 1;
  static const int kNextState = kNumInputs + 1;
  static const int kStateSize = (kNumInputs - 1) * (kNumInputs - 1);
  static constexpr int Symmetry(int i) { return (i < N) ? 0 : i; }
  static constexpr int Bitsize(int i) {
    return (i == N || i == N + 2) ? nldtool::Log2(N - 1) + 1 : 1;
  }
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    y[0] = std::accumulate(&x[1], &x[kNumInputs], x[0], std::plus<T>());
    y[1] = y[0] >> 1;
    y[0] &= 1;
  }
};

template <int N>
constexpr char ADD<N>::kName[];

/*!
 * \brief Template class implementing a modular addition with N inputs and 2-bit
 * constraints.
 *
 * Like ADD, but takes twobit conditions into account. For a detailed
 * explanation see Gaëtan Leurent: Analysis of Differential Attacks in ARX
 * Constructions. ASIACRYPT 2012 http://dx.doi.org/10.1007/978-3-642-34961-4_15
 */
template <int N>
class ADDB : public F {
 public:
  static constexpr char kName[] = "ADDB<N>";
  static const int kNumInputs = N + 1;
  static const int kNumOutputs = 3;
  static constexpr int Symmetry(int i) { return (i < N) ? 0 : i; }
  static constexpr int Bitsize(int i) {
    return (i == N || i == N + 2 || i == N + 3) ? nldtool::Log2(N - 1) + 1 : 2;
  }
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    const int ci = kNumInputs - 1;
    const int s = 0;
    const int cm = 1;
    const int co = 2;
    auto add_lsb = [](T acc, T d) { return acc + (d & 1); };
    y[cm] = std::accumulate(&x[0], &x[ci], x[ci], add_lsb) >> 1;
    y[s] = std::accumulate(&x[1], &x[kNumInputs], x[0], std::plus<T>());
    y[co] = y[0] >> 2;
    y[s] &= 3;
  }
};

template <int N>
constexpr char ADDB<N>::kName[];

#endif  // FUNCTIONS_H_

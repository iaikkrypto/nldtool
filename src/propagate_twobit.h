#ifndef PROPAGATE_TWOBIT_H_
#define PROPAGATE_TWOBIT_H_

#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

#include "bitslice_data.h"
#include "propagate_twobit_output.h"
#include "utils.h"

//#define DIFFTWOBIT

/*!
 * \brief Like \see Propagate, but for TwobitConditions
 */
template <class F>
class PropagateTwobit {
 public:
  typedef BitsliceData<F> Input;
  typedef PropagateTwobitOutput Output;

  void Initialize(Input input) {
    twobits_.clear();
    int num = F::kNumInputs + F::kNumOutputs;
    twobits_.reserve(num * num);
    uint64_t bca, bcb;
    for (uint8_t a = 1; a < num; a++) {
      bca = input.GetCondition(a);
      if (nldtool::HammingWeight(bca) == 1) continue;
      for (uint8_t b = 0; b < a; b++) {
        bcb = input.GetCondition(b);
        if (nldtool::HammingWeight(bcb) == 1) continue;
        if (F::Bitsize(a) == 1 && F::Bitsize(b) == 1)
          if (bca != bcb) continue;
        twobits_.push_back({a, b, 0});
      }
    }

#ifdef DEBUG_TWOBIT
    unsigned char bc[16] = {'#', '0', 'u', '3', 'n', '5', 'x', '7',
                            '1', '-', 'A', 'B', 'C', 'D', 'E', '?'};
    if (twobits_.size() > 0) {
      std::cout << "ind: ";
      for (int i = F::kNumInputs + F::kNumOutputs - 1; i >= 0; --i) {
        std::cout << std::hex << i << std::dec;
        if (i != 0) std::cout << ".";
      }
      std::cout << std::endl;
      std::cout << "in:  ";
      for (int i = F::kNumInputs + F::kNumOutputs - 1; i >= 0; --i) {
        if (F::Bitsize(i) == 1)
          std::cout << bc[input.GetCondition(i)];
        else
          std::cout << "X";
        if (i != 0) std::cout << ".";
      }
      std::cout << "  " << input;
      std::cout << std::endl;
    }
#endif
  }

  void Collect(BitslicePair<F> val) {
    for (int i = 0; i < F::kNumInputs + F::kNumOutputs; ++i) {
      if (F::Bitsize(i) != 1) {
        const uint64_t a = val[i].first;
        const uint64_t b = val[i].second;
        uint64_t v = 0;
        if ((a & 1) == (b & 1))
          v = (a & 1) | ((b & 1) << 1);
        else
          v = 1ull << (b < a);
        val[i].first = v & 1;
        val[i].second = (v >> 1) & 1;
      }
    }

#ifdef DEBUG_TWOBIT
    unsigned char bc[4] = {'0', 'u', 'n', '1'};
    if (twobits_.size() > 0) {
      for (int i = F::kNumInputs + F::kNumOutputs - 1; i >= 0; --i) {
        uint64_t v = (val[i].second << 1) | val[i].first;
        if ((v & 3) == v)
          std::cout << bc[v];
        else
          std::cout << "*";
        if (i != 0) std::cout << ".";
      }
      std::cout << "  ";
    }
#endif

    // update TwobitConditions
    for (int i = 0; i < twobits_.size(); i++) {
      BitsliceTwobitCondition twobit = twobits_[i];
      const int a = twobit.a;
      const int b = twobit.b;
      const uint64_t va = (val[a].second << 1) | val[a].first;
      const uint64_t vb = (val[b].second << 1) | val[b].first;

      // x  x'
      // \  /
      //  00: both equal
      //  01: equal / not equal
      //  10: not equal / equal
      //  11: both not equal
      const uint64_t ab = (va ^ vb) & 0x3ull;
      //              (x,x')^(y,y')

      // ..00: contradiction
      // ..01: equal
      // ..10: not equal
      // ..11: equal or not equal

      // 00..: contradiction
      // 01..: diff equal
      // 10..: diff not equal
      // 11..: equal or not equal

      twobits_[i].c |=
          (ab != 0x3ull ? 1ull : 0ull) | ((ab != 0x0ull ? 1ull : 0ull) << 1);

#ifdef DEBUG_TWOBIT
      std::cout << "(" << (int)twobits_[i].a << "," << (int)twobits_[i].b
                << ")=" << (int)twobits_[i].c << " ";
//      std::cout << std::hex << ((int)(twobits_[i].a)) <<
//      ((int)(twobits_[i].b)) << ((ab != 0x3ull ? 1ull : 0ull) | ((ab != 0x0ull
//      ? 1ull : 0ull) << 1)) << std::dec << ",";
#endif

#ifdef DIFFTWOBIT
      if (ab == 1 || ab == 2)
        twobits_[i].c |= 8;
      else
        twobits_[i].c |= 4;
#endif

      // example twobitcond with two inputs for: x1 + x2 = su
      //
      //        4  3  2  1  0   ... index
      //       co ci su x2 x1   ... input or output
      // val2: 00 10 11 00 01   ... value for (b,a)
      //
      //  43 42 41 40 32 31 30 21 20 10   ... indices which are compared
      //  11 10 01 11 11 11 10 10 11 11   ... resulting twobitcond (10: not
      //  equal, 01: equal)
    }
#ifdef DEBUG_TWOBIT
#ifndef DEBUG_LOOP
    if (twobits_.size()) std::cout << std::endl;
#endif
#endif
  }

  Output Finalize(int64_t size) {
    Output output;
    output.twobit_list_.clear();
    output.twobit_list_.reserve(twobits_.size());
    BitsliceTwobitCondition t;
#ifdef DEBUG_TWOBIT
    if (twobits_.size() > 0) std::cout << "twobit: ";
#endif
    for (int i = 0; i < twobits_.size(); i++) {
#ifdef DEBUG_TWOBIT
      std::cout << "(" << ((int)(twobits_[i].a)) << ","
                << ((int)(twobits_[i].b)) << ")=" << ((int)(twobits_[i].c))
                << " ";
#endif
      // add (diff) twobit conditions
      t = twobits_[i];
      t.c &= 0x3;
      if ((t.c == 1) || (t.c == 2)) {
        output.twobit_list_.push_back(t);
      }
#ifdef DIFFTWOBIT
      // add differential twobit conditions
      t = twobits_[i];
      t.c &= 0xc;
      if ((t.c == 4) || (t.c == 8)) {
        output.twobit_list_.push_back(t);
      }
#endif
    }
#ifdef DEBUG_TWOBIT
    if (twobits_.size() > 0) std::cout << std::endl;
#endif
    return output;
  }

  static std::string GetName() { return "PropagateTwobit"; }

  std::vector<BitsliceTwobitCondition> twobits_;
};

#endif  // PROPAGATE_TWOBIT_H_

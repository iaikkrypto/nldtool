#ifndef BITSLICE_PAIR_H_
#define BITSLICE_PAIR_H_

#include <cstdint>
#include <ostream>

#include "pair.h"

/*!
 * \brief An array of \see Pair with the required length for the combined in-
 * and outputs of the templated function.
 */
template <class F>
class BitslicePair {
 public:
  Pair x[F::kNumInputs + F::kNumOutputs];

  Pair& operator[](int i) { return x[i]; }

  Pair operator[](int i) const { return x[i]; }

  uint64_t GetPair(int i) const {
    return 1ull << ((x[i].second << F::Bitsize(i)) | x[i].first);
  }

  friend std::ostream& operator<<(std::ostream& os, const BitslicePair& bs) {
    os << "(";
    // second output
    for (int j = F::kNumOutputs - 1; j >= 0; --j) {
      int a = F::kNumInputs + j;
      for (int i = F::Bitsize(a) - 1; i >= 0; --i)
        os << ((bs[a].second >> i) & 1);
      if (j != 0) os << ".";
    }
    os << "=";
    // second input
    for (int j = F::kNumInputs - 1; j >= 0; --j) {
      for (int i = F::Bitsize(j) - 1; i >= 0; --i)
        os << ((bs[j].second >> i) & 1);
      if (j != 0) os << ".";
    }
    os << ", ";
    // first output
    for (int j = F::kNumOutputs - 1; j >= 0; --j) {
      int a = F::kNumInputs + j;
      for (int i = F::Bitsize(a) - 1; i >= 0; --i)
        os << ((bs[a].first >> i) & 1);
      if (j != 0) os << ".";
    }
    os << "=";
    // first input
    for (int j = F::kNumInputs - 1; j >= 0; --j) {
      for (int i = F::Bitsize(j) - 1; i >= 0; --i)
        os << ((bs[j].first >> i) & 1);
      if (j != 0) os << ".";
    }
    os << ")";
    return os;
  }
};

#endif  // BITSLICE_PAIR_H_

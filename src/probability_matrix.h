#ifndef PROBABILITY_MATRIX_H_
#define PROBABILITY_MATRIX_H_

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

#include "bitslice_data.h"
#include "probability_output_matrix.h"
#include "utils.h"

/*!
 * \brief Advanced calculation of probability using matrices for the
 * CYCLICGRAPH probability method
 */
template <class F>
class ProbabilityMatrix {
 public:
  typedef BitsliceData<F> Input;
  typedef ProbabilityOutputMatrix Output;

  void Initialize(Input input) {
    input_ = input;
    output_ = Output(F::kStateSize);
  }

  void Collect(BitslicePair<F> val) {
    uint64_t a = val[F::kPrevState].first;
    uint64_t b = val[F::kPrevState].second;
    uint64_t c0 = (b << F::Bitsize(F::kPrevState)) | a;
    a = val[F::kNextState].first;
    b = val[F::kNextState].second;
    uint64_t c1 = (b << F::Bitsize(F::kNextState)) | a;
    c0 = state_reordering_[F::Bitsize(F::kPrevState) - 1][c0];
    c1 = state_reordering_[F::Bitsize(F::kPrevState) - 1][c1];
    if (c1 < F::kStateSize && c0 < F::kStateSize) output_.Increment(c1, c0);
  }

  Output Finalize(int64_t size) {
    uint64_t total_possibilities = 1;
    for (int i = 0; i < F::kNumInputs; ++i) {
      if (i == F::kPrevState) continue;
      uint64_t cond = input_.GetCondition(i);
      uint64_t possibilities = 0;
      for (int j = 0; j < sizeof(uint64_t); ++j) {
        if ((cond & 1) == 1) possibilities++;
        cond = cond >> 1;
      }
      total_possibilities *= possibilities;
    }
    output_.Divide((float)total_possibilities);
    return output_;
  }

  static std::string GetName() { return "ProbabilityMatrix"; }

  Input input_;
  Output output_;
  unsigned int state_reordering_[3][64] = {
      {0, 1, 2, 3},
      {0, 1, 4, 9, 2, 3, 6, 11, 5, 7, 8, 13, 10, 12, 14, 15},
      {0,  1,  4,  9,  16, 25, 36, 49, 2,  3,  6,  11, 18, 27, 38, 51,
       5,  7,  8,  13, 20, 29, 40, 53, 10, 12, 14, 15, 22, 31, 42, 55,
       17, 19, 21, 23, 24, 33, 44, 57, 26, 28, 30, 32, 34, 35, 46, 59,
       37, 39, 41, 43, 45, 47, 48, 61, 50, 52, 54, 56, 58, 60, 62, 63}};
};

#endif  // PROBABILITY_MATRIX_H_

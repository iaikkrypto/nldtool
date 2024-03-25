#ifndef PROBABILITY_H_
#define PROBABILITY_H_

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

#include "bitslice_data.h"
#include "probability_output.h"
#include "utils.h"

/*!
 * \brief Calculates the probability of the given in- and output conditions to
 * occur by collecting all valid pairs.
 */
template <class F>
class Probability {
 public:
  typedef BitsliceData<F> Input;
  typedef ProbabilityOutput Output;

  void Initialize(Input input) { pairs_ = 0; }

  void Collect(BitslicePair<F> val) { pairs_++; }

  Output Finalize(int64_t size) {
    return {(float)(log((float)pairs_ / (float)size) / log(2.0))};
  }

  static std::string GetName() { return "Probability"; }

  int64_t pairs_;
};

#endif  // PROBABILITY_H_

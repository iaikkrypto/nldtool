#ifndef PROPAGATE_H_
#define PROPAGATE_H_

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

#include "bitslice_data.h"
#include "utils.h"

/*!
 * \brief Calculates the condition output for a given condition input data by
 * combining all possible outputs
 */
template <class F>
class Propagate {
 public:
  typedef BitsliceData<F> Input;
  typedef BitsliceData<F> Output;

  void Initialize(Input input) { input_ = input; }

  void Collect(BitslicePair<F> val) {
    for (int i = 0; i < F::kNumInputs + F::kNumOutputs; ++i) {
      uint64_t cond = output_.GetCondition(i);
      cond |= val.GetPair(i);
      output_.SetCondition(i, cond);
    }
  }

  Output Finalize(int64_t size) { return output_; }

  static std::string GetName() { return "Propagate"; }

  Input input_;
  Output output_;
};

#endif  // PROPAGATE_H_

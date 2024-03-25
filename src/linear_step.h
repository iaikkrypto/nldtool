#ifndef LINEAR_STEP_H_
#define LINEAR_STEP_H_

#include <cassert>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <set>
#include <vector>

#include "bitpos.h"
#include "characteristic.h"
#include "condition_word.h"
#include "linear_conditions.h"
#include "linear_step_data.h"
#include "step.h"

//#define DEBUG_LINEARSTEP
//#define TRANSFER_LINEAR_CONDITIONS

/*!
 * \brief A description of a linear step.
 *
 * If a function in the crypto definition can be composed of linear operations,
 * this LinearStep can be used instead of a \see BitsliceStep. This allows for
 * many optimizations like undoing the step if a contradiction is reached and
 * faster computation of results just solving a system of equations instead of
 * brute-forcing all possibilities like in the \see BitsliceStep.
 */
template <class F>
class LinearStep : public Step {
 public:
  template <class... Types>
  LinearStep(Types... rest) : Step(rest...) {}

  LinearStep(int word_size, const std::vector<ConditionWordPtr>& params)
      : Step(word_size, params) {}

  virtual void Init(int step_index, Priority priority) {
    step_index_ = step_index;
    priority_ = priority.mul * word_size_ * params_.size() + priority.add;
    // std::cout << GetName() << " priority: " << priority_ << std::endl;
    for (int p = 0; p < GetNumParams(); ++p)
      for (int bit = 0; bit < word_size_; ++bit) {
        ConditionProxyPtr cp = GetParam(p)->GetConditionProxy(bit);
        const int bit_index = (params_.size() - 1 - p) * word_size_ + bit;
        StepUpdate pos;
        pos.priority = priority_;
        pos.step = step_index_;
        pos.bit = bit_index;
        cp->AddStepToUpdate(pos);
      }
  }

  virtual ~LinearStep() {}

  virtual std::string GetName() const { return std::string("LinearStep"); }

  virtual StepData* CreateStepData() const {
    assert(F::kNumInputs + F::kNumOutputs == params_.size());
    LinearStepData* data =
        new LinearStepData(F::kNumOutputs * word_size_,
                           (F::kNumInputs + F::kNumOutputs) * word_size_);
    data->GetLinearConditions().InitLinearFunction<F>(word_size_);
    return data;
  }

  virtual bool Update(Characteristic& characteristic,
                      std::set<Bitpos>& condition_pos, int step) const {
    return true;
  }

  virtual bool Update(Characteristic& characteristic, int pos,
                      bool backtrack = false) const {
    LinearConditions& linear_conditions =
        static_cast<LinearStepData&>(characteristic.GetStepData(step_index_))
            .GetLinearConditions();
#ifdef DEBUG_LINEARSTEP
    std::cout << "LS"
              << "(" << step_index_ << "): " << pos << std::endl;
//    linear_conditions.PrintMatrix();
#endif
    ConditionProxyPtr cp =
        params_[params_.size() - 1 - pos / word_size_]->GetConditionProxy(
            pos % word_size_);
    BitCondition bit_condition = BitCondition(cp->GetCondition(characteristic));
    assert(bit_condition != BitCondition("#"));
    if (!linear_conditions.AddLinearConditions(bit_condition, pos, backtrack)) {
      return false;
    }
#ifdef DEBUG_LINEARSTEP
//    linear_conditions.PrintMatrix();
#endif

    return true;
  }

  virtual bool PropagateConditions(Characteristic& characteristic) const {
    LinearConditions& linear_conditions =
        static_cast<LinearStepData&>(characteristic.GetStepData(step_index_))
            .GetLinearConditions();
    if (!linear_conditions.PropagateConditions(characteristic, params_,
                                               word_size_)) {
      return false;
    }
#ifdef TRANSFER_LINEAR_CONDITIONS
    for (Overlap overlap : overlap_) {
      const int other_step_index = overlap.other_step->GetStepIndex();
      LinearConditions& other_linear_conditions =
          static_cast<LinearStepData&>(
              characteristic.GetStepData(other_step_index))
              .GetLinearConditions();
#ifdef DEBUG_TRANSFER
      std::cout << "TransferLinearConditions " << step_index_ << "->"
                << other_step_index << std::endl;
#endif
      if (!linear_conditions.TransferLinearConditions(other_linear_conditions,
                                                      overlap, word_size_)) {
        return false;
      }
    }
#endif
    return true;
  }

  int getNumEq(BitCondition bc) const {
    switch (bc.GetChar()) {
      case '#':
        return -1;
      case '0':
        return 2;
      case '1':
        return 2;
      case 'n':
        return 2;
      case 'u':
        return 2;
      case '-':
        return 1;
      case 'x':
        return 1;
      case '3':
        return 1;
      case '5':
        return 1;
      case 'A':
        return 1;
      case 'C':
        return 1;
    }
    return 0;  // other eqs?
  }

  virtual float GetProbability(const Characteristic& characteristic) const {
    float probability = 0.0;
    const LinearConditions& linear_conditions =
        static_cast<const LinearStepData&>(
            characteristic.GetStepData(step_index_))
            .GetLinearConditions();

    int f = linear_conditions.initial_f_count;
    int eq_in = 0;
    int eq_out = 0;

    for (int i = 0; i < F::kNumInputs; i++) {
      for (int j = 0; j < word_size_; j++) {
        int eq = getNumEq(
            GetParam(i)->GetConditionProxy(j)->GetCondition1(characteristic));
        if (eq == -1) {
          return -std::numeric_limits<float>::infinity();
        }
        eq_in += eq;
      }
    }
    for (int i = 0; i < F::kNumOutputs; i++) {
      for (int j = 0; j < word_size_; j++) {
        eq_out += getNumEq(GetParam(F::kNumInputs + i)
                               ->GetConditionProxy(j)
                               ->GetCondition1(characteristic));
      }
    }

    probability =
        (f + eq_in) -
        (linear_conditions.GetMatrix().GetPivotRowCount() + eq_in + eq_out);
    return probability;
  }

  virtual std::vector<TwobitCondition> UpdateTwobitCondition(
      Characteristic& characteristic, Bitpos pos) const {
    return std::vector<TwobitCondition>();
  }
};

#endif  // LINEAR_STEP_H_

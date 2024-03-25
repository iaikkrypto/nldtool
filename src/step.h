#ifndef STEP_H_
#define STEP_H_

#include <cstdint>
#include <vector>

#include "bitpos.h"
#include "condition_word.h"
#include "twobit_condition.h"

class Characteristic;
class StepData;

struct Overlap {
  int start;
  int size;
  Step* other_step;
  int other_start;
};

/*!
 * \brief Base class for \see BitsliceStep and \see LinearStep
 *
 * Provides the common functionality for the two step types.
 */
class Step {
 public:
  struct Priority {
    int mul;
    int add;
  };

  void AddParams() {}

  template <class T, class... Args>
  void AddParams(T param, Args... args) {
    params_.push_back(param);
    AddParams(args...);
  }

  template <class... Args>
  Step(int word_size, Args... args)
      : word_size_(word_size),
        step_index_(-1),
        priority_(-1),
        probability_method_(0) {
    AddParams(args...);
  }

  Step(int word_size, std::vector<ConditionWordPtr> params)
      : params_(params),
        word_size_(word_size),
        step_index_(-1),
        priority_(-1),
        probability_method_(0) {}

  virtual ~Step() {}

  Step(const Step& s)
      : params_(s.params_),
        word_size_(s.word_size_),
        step_index_(s.step_index_),
        priority_(s.priority_),
        probability_method_(0) {}

  Step& operator=(const Step& s) {
    params_ = s.params_;
    word_size_ = s.word_size_;
    return *this;
  }

  virtual bool IsCarryStep() { return false; }

  virtual StepData* CreateStepData() const = 0;

  virtual void Init(int step_index, Priority priority) = 0;

  virtual void AddOverlap(Overlap overlap) { overlap_.push_back(overlap); }

  const ConditionWordPtr GetParam(int i) const { return params_[i]; }

  void AddParam(ConditionWordPtr a) { params_.push_back(a); }

  void PopParam() { params_.pop_back(); }

  int GetNumParams() const { return params_.size(); }

  int GetStepIndex() const { return step_index_; }

  virtual std::string GetName() const { return ""; }

  virtual ConditionProxyPtr GetConditionProxy(Bitpos pos) const {
    return GetParam(pos.GetWord())->GetConditionProxy(pos.GetBit());
  }

  virtual ConditionProxyPtr GetConditionProxy(int param, int bit) const {
    return GetParam(param)->GetConditionProxy(bit);
  }

  virtual bool Update(Characteristic& characteristic, int pos,
                      bool backtrack = false) const = 0;

  virtual float GetProbability(const Characteristic& characteristic) const {
    return 0.0;
  }

  virtual std::vector<TwobitCondition> UpdateTwobitCondition(
      Characteristic& characteristic, Bitpos bitslice_pos) const = 0;

  virtual bool PropagateConditions(Characteristic& characteristic) const {
    return true;
  }

  void SetProbabilityMethod(int method) { probability_method_ = method; }

 protected:
  std::vector<ConditionWordPtr> params_;
  std::vector<Overlap> overlap_;
  int word_size_;
  int step_index_;
  int priority_;
  int probability_method_;
};

#endif  // STEP_H_

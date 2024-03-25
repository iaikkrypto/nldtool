#ifndef LINEAR_STEP_DATA_H_
#define LINEAR_STEP_DATA_H_

#include "linear_conditions.h"
#include "step_data.h"

/*!
 * \brief Class containing the data for a \see LinearStep (\see
 * LinearConditions)
 */
class LinearStepData : public StepData {
 public:
  LinearStepData(int rows, int cols) : linear_conditions_(rows, cols) {}
  LinearStepData(const LinearStepData& cp)
      : linear_conditions_(cp.linear_conditions_) {}
  virtual ~LinearStepData() {}
  LinearStepData& operator=(const LinearStepData& cp) {
    linear_conditions_ = cp.linear_conditions_;
    return *this;
  }
  virtual StepData* Clone() const { return new LinearStepData(*this); }
  virtual void Copy(StepData** data) const {
    ((LinearStepData*)(*data))->linear_conditions_ = linear_conditions_;
  }
  virtual void ShallowCopy(const StepData& data) {
    linear_conditions_.ShallowCopy(
        ((LinearStepData*)(&data))->linear_conditions_);
  }
  virtual void Undo() { linear_conditions_.Undo(); }
  virtual void UndoAndRestore() { linear_conditions_.UndoAndRestore(); }
  const LinearConditions& GetLinearConditions() const {
    return linear_conditions_;
  }
  LinearConditions& GetLinearConditions() { return linear_conditions_; }
  void Print() const { linear_conditions_.Print(); }

 protected:
  LinearConditions linear_conditions_;
};

#endif  // LINEAR_STEP_DATA_H_

#ifndef STEP_DATA_H_
#define STEP_DATA_H_

/*!
 * \brief Abstract base class describing the possible actions on a \see
 * StepData.
 */
class StepData {
 public:
  virtual ~StepData() {}
  virtual StepData* Clone() const = 0;
  virtual void Copy(StepData** data) const = 0;
  virtual void ShallowCopy(const StepData& data) {}
  virtual void Undo() {}
  virtual void UndoAndRestore() {}
  virtual void Print() const {};
};

#endif  // STEP_DATA_H_

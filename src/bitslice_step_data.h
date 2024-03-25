#ifndef BITSLICE_STEP_DATA_H_
#define BITSLICE_STEP_DATA_H_

#include "step_data.h"

/*!
 * \brief An empty data object, since we do not need additional data for a \see
 * BitsliceStep.
 */
class BitsliceStepData : public StepData {
 public:
  virtual ~BitsliceStepData() {}
  virtual StepData* Clone() const { return new BitsliceStepData(*this); }
  virtual void Copy(StepData** data) const {}
};

#endif  // BITSLICE_STEP_DATA_H_

#ifndef CARRY_STEP_H_
#define CARRY_STEP_H_

#include "bitslice_step.h"

/*!
 * \brief Like a BitsliceStep, but when used with \see ADD, the needed carry
 * words are added automatically
 */
template <class F>
class CarryStep : public BitsliceStep<F> {
 public:
  template <class... Types>
  CarryStep(Types... rest) : BitsliceStep<F>(rest...) {}

  virtual ~CarryStep() {}

  virtual bool IsCarryStep() { return true; }
};

#endif  // CARRY_STEP_H_

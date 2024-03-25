#ifndef STEP_UPDATE_H_
#define STEP_UPDATE_H_

#include <cassert>
#include <cstdint>

struct StepUpdate {
  int32_t priority;
  int16_t step;
  int16_t bit;
  bool operator<(const StepUpdate& rhs) const {
    assert(priority >= 0 && step >= 0 && bit >= 0);
    assert(rhs.priority >= 0 && rhs.step >= 0 && rhs.bit >= 0);
    return (uint64_t(priority) << 32 | uint64_t(step) << 16 | uint64_t(bit)) <
           (uint64_t(rhs.priority) << 32 | uint64_t(rhs.step) << 16 |
            uint64_t(rhs.bit));
  }
};

#endif  // STEP_UPDATE_H_

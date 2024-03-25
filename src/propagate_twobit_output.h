#ifndef PROPAGATE_TWOBIT_OUTPUT_H_
#define PROPAGATE_TWOBIT_OUTPUT_H_

#include <cstdint>
#include <iostream>
#include <vector>

#include "bitslice_twobit_condition.h"

/*!
 * \brief Data structure for the output of the \see PropagateTwobit action
 */
class PropagateTwobitOutput {
 public:
  std::vector<BitsliceTwobitCondition> twobit_list_;
  friend std::ostream& operator<<(std::ostream& os, PropagateTwobitOutput& t);
  void ResortOutput(uint8_t perm[], int len);
  const char* GetBytePtr() const;
  int GetByteSize() const;
  void SetFromBytePtr(const char* data, int size);
};

#endif  // PROPAGATE_TWOBIT_OUTPUT_H_

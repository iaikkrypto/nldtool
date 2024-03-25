#ifndef PROBABILITY_OUTPUT_H_
#define PROBABILITY_OUTPUT_H_

#include <cstdint>
#include <iostream>

/*!
 * \brief Data structure for the output of the \see Probability action
 */
class ProbabilityOutput {
 public:
  float probability_;

  friend std::ostream& operator<<(std::ostream& os, ProbabilityOutput& t);
  void ResortOutput(uint8_t perm[], int len);
  const char* GetBytePtr() const;
  int GetByteSize() const;
  void SetFromBytePtr(const char* data, int size);
};

#endif  // PROBABILITY_OUTPUT_H_

#include "probability_output.h"

#include <cstring>

std::ostream& operator<<(std::ostream& os, ProbabilityOutput& t) {
  os << t.probability_;
  return os;
}

void ProbabilityOutput::ResortOutput(uint8_t perm[], int len) {}

const char* ProbabilityOutput::GetBytePtr() const {
  return reinterpret_cast<const char*>(&probability_);
}

int ProbabilityOutput::GetByteSize() const { return sizeof(probability_); }

void ProbabilityOutput::SetFromBytePtr(const char* data, int size) {
  // assert(size == sizeof(WordContainer<GETWORDS>));
  memcpy(&probability_, data, size);
}

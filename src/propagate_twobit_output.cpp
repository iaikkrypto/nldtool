#include "propagate_twobit_output.h"

std::ostream& operator<<(std::ostream& os, PropagateTwobitOutput& t) {
  for (int i = 0; i < t.twobit_list_.size(); i++) {
    os << t.twobit_list_[i];
    if (i != t.twobit_list_.size() - 1) os << ",";
  }
  return os;
}

void PropagateTwobitOutput::ResortOutput(uint8_t perm[], int len) {
  std::vector<BitsliceTwobitCondition> old_list(twobit_list_);
  for (int i = 0; i < twobit_list_.size(); i++) {
    assert(twobit_list_[i].c == 1 || twobit_list_[i].c == 2 ||
           twobit_list_[i].c == 4 || twobit_list_[i].c == 8);
    twobit_list_[i].a = perm[old_list[i].a];
    twobit_list_[i].b = perm[old_list[i].b];
  }
}

const char* PropagateTwobitOutput::GetBytePtr() const {
  return reinterpret_cast<const char*>(twobit_list_.data());
}

int PropagateTwobitOutput::GetByteSize() const {
  return twobit_list_.size() * sizeof(BitsliceTwobitCondition);
}

void PropagateTwobitOutput::SetFromBytePtr(const char* data, int size) {
  assert(size % sizeof(BitsliceTwobitCondition) == 0);
  int count = size / sizeof(BitsliceTwobitCondition);
  BitsliceTwobitCondition* ptr =
      reinterpret_cast<BitsliceTwobitCondition*>(const_cast<char*>(data));
  twobit_list_.assign(ptr, ptr + count);
}

#include "konstant_condition_proxy.h"

#include <sstream>

KonstantConditionProxy::KonstantConditionProxy(int num_bits, uint64_t condition)
    : ConditionProxy(num_bits), condition_(condition) {}

KonstantConditionProxy::~KonstantConditionProxy() {}

int KonstantConditionProxy::IsPosKonstant(int pos) const { return 1; }

std::string KonstantConditionProxy::ToString() const {
  std::ostringstream oss;
  oss << "K"
      << "(" << (condition_ == 1 ? 0 : 1) << ")";
  return oss.str();
}

uint64_t KonstantConditionProxy::GetCondition(
    const Characteristic& characteristic) const {
  return condition_;
}

Condition<1> KonstantConditionProxy::GetCondition1(
    const Characteristic& characteristic, int start) const {
  if (num_bits_ == 1 && start == 0)
    return Condition<1>(condition_);
  else if (num_bits_ == 2 && start == 0)
    return Condition<2>(condition_).GetBit0();
  else if (num_bits_ == 2 && start == 1)
    return Condition<2>(condition_).GetBit1();
  else if (num_bits_ == 3 && start == 0)
    return Condition<3>(condition_).GetBit0();
  else if (num_bits_ == 3 && start == 1)
    return Condition<3>(condition_).GetBit1();
  else if (num_bits_ == 3 && start == 2)
    return Condition<3>(condition_).GetBit2();
  assert(!"bit index error");
  return Condition<1>("#");
}

Condition<2> KonstantConditionProxy::GetCondition2(
    const Characteristic& characteristic, int start) const {
  if (num_bits_ == 2 && start == 0)
    return Condition<2>(condition_);
  else if (num_bits_ == 3 && start == 0)
    return Condition<3>(condition_).GetBits01();
  else if (num_bits_ == 3 && start == 1)
    return Condition<3>(condition_).GetBits12();
  assert(!"bit index error");
  return 0;
}

Condition<3> KonstantConditionProxy::GetCondition3(
    const Characteristic& characteristic, int start) const {
  if (num_bits_ == 3 && start == 0) return Condition<3>(condition_);
  assert(!"bit index error");
  return 0;
}

bool KonstantConditionProxy::SetCondition(Characteristic& characteristic,
                                          uint64_t value) const {
  return (condition_ & value) == value;
}

bool KonstantConditionProxy::SetCondition1(Characteristic& characteristic,
                                           Condition<1> cond, int start) const {
  if (num_bits_ == 1 && start == 0)
    return Condition<1>(condition_).Filter(cond);
  else if (num_bits_ == 2 && start == 0)
    return Condition<2>(condition_).GetBit0().Filter(cond);
  else if (num_bits_ == 2 && start == 1)
    return Condition<2>(condition_).GetBit1().Filter(cond);
  else if (num_bits_ == 3 && start == 0)
    return Condition<3>(condition_).GetBit0().Filter(cond);
  else if (num_bits_ == 3 && start == 1)
    return Condition<3>(condition_).GetBit1().Filter(cond);
  else if (num_bits_ == 3 && start == 2)
    return Condition<3>(condition_).GetBit2().Filter(cond);
  assert(!"bit index error");
  return 0;
}

bool KonstantConditionProxy::SetCondition2(Characteristic& characteristic,
                                           Condition<2> cond, int start) const {
  if (num_bits_ == 2 && start == 0)
    return Condition<2>(condition_).Filter(cond);
  else if (num_bits_ == 3 && start == 0)
    return Condition<3>(condition_).GetBits01().Filter(cond);
  else if (num_bits_ == 3 && start == 1)
    return Condition<3>(condition_).GetBits12().Filter(cond);
  assert(!"bit index error");
  return 0;
}

bool KonstantConditionProxy::SetCondition3(Characteristic& characteristic,
                                           Condition<3> cond, int start) const {
  if (num_bits_ == 3 && start == 0)
    return Condition<3>(condition_).Filter(cond);
  assert(!"bit index error");
  return 0;
}

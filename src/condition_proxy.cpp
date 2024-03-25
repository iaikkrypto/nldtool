#include "condition_proxy.h"

#include <ostream>

#include "characteristic.h"

ConditionProxy::ConditionProxy(int num_bits) : num_bits_(num_bits) {
  assert(num_bits_ != 0);
}

ConditionProxy::~ConditionProxy() {}

int ConditionProxy::GetHammingWeight(
    const Characteristic& characteristic) const {
  return nldtool::HammingWeight(GetCondition(characteristic));
}

Condition<1> ConditionProxy::GetCondition1(const Characteristic& characteristic,
                                           int start) const {
  assert(!"GetCondition1");
  return Condition<1>("#");
}
Condition<2> ConditionProxy::GetCondition2(const Characteristic& characteristic,
                                           int start) const {
  assert(!"GetCondition2");
  return 0;
}

Condition<3> ConditionProxy::GetCondition3(const Characteristic& characteristic,
                                           int start) const {
  assert(!"GetCondition3");
  return 0;
}

bool ConditionProxy::SetCondition1(Characteristic& characteristic,
                                   Condition<1> condition, int start) const {
  assert(!"SetCondition1");
  return 0;
}

bool ConditionProxy::SetCondition2(Characteristic& characteristic,
                                   Condition<2> condition, int start) const {
  assert(!"SetCondition2");
  return 0;
}

bool ConditionProxy::SetCondition3(Characteristic& characteristic,
                                   Condition<3> condition, int start) const {
  assert(!"SetCondition3");
  return 0;
}

void ConditionProxy::AddStepToUpdate(StepUpdate step) {}

const std::set<StepUpdate> ConditionProxy::GetStepsToUpdate() const {
  return std::set<StepUpdate>();
}

const std::vector<Bitpos> ConditionProxy::GetBitposList() const {
  return std::vector<Bitpos>();
}

std::string ConditionProxy::ToString() const { return "ConditionProxyBase"; }

int ConditionProxy::GetNumBits() const { return num_bits_; }

int ConditionProxy::IsPosKonstant(int pos) const { return -1; }

bool ConditionProxy::IsMaskConditionProxy() const { return false; }

Bitpos ConditionProxy::GetConditionMaskPos() const { return Bitpos(-1, -1); }

std::ostream& operator<<(std::ostream& os, ConditionProxy& condition_proxy) {
  os << condition_proxy.ToString();
  return os;
}

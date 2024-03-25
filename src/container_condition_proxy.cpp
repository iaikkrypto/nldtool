#include "container_condition_proxy.h"

#include <sstream>

#include "characteristic.h"

ContainerConditionProxy::ContainerConditionProxy(int num_bits,
                                                 Bitpos condition_mask_pos,
                                                 Bitpos container_pos)
    : ConditionProxy(num_bits),
      MaskConditionProxy(num_bits, condition_mask_pos),
      container_pos_(container_pos) {}

ContainerConditionProxy::~ContainerConditionProxy() {}

int ContainerConditionProxy::IsPosKonstant(int pos) const { return 0; }

void ContainerConditionProxy::AddStepToUpdate(StepUpdate step) {
  step_update_list_.insert(step);
}

const std::set<StepUpdate> ContainerConditionProxy::GetStepsToUpdate() const {
  return step_update_list_;
}

Bitpos ContainerConditionProxy::GetContainerPos() const {
  return container_pos_;
}

std::string ContainerConditionProxy::ToString() const {
  std::ostringstream oss;
  oss << "S" << num_bits_ << container_pos_ << condition_mask_pos_;
  return oss.str();
}

uint64_t ContainerConditionProxy::GetCondition(
    const Characteristic& characteristic) const {
  if (num_bits_ == 1)
    return characteristic.GetContainerCondition1(container_pos_);
  else if (num_bits_ == 2)
    return characteristic.GetContainerCondition2(container_pos_);
  else if (num_bits_ == 3)
    return characteristic.GetContainerCondition3(container_pos_);
  assert(!"invalid number of bits");
  return 0;
}

Condition<1> ContainerConditionProxy::GetCondition1(
    const Characteristic& characteristic, int start) const {
  if (num_bits_ == 1 && start == 0)
    return characteristic.GetContainerCondition1(container_pos_);
  else if (num_bits_ == 2 && start == 0)
    return characteristic.GetContainerCondition2(container_pos_).GetBit0();
  else if (num_bits_ == 2 && start == 1)
    return characteristic.GetContainerCondition2(container_pos_).GetBit1();
  else if (num_bits_ == 3 && start == 0)
    return characteristic.GetContainerCondition3(container_pos_).GetBit0();
  else if (num_bits_ == 3 && start == 1)
    return characteristic.GetContainerCondition3(container_pos_).GetBit1();
  else if (num_bits_ == 3 && start == 2)
    return characteristic.GetContainerCondition3(container_pos_).GetBit2();
  assert(!"bit index error");
  return Condition<1>("#");
}

Condition<2> ContainerConditionProxy::GetCondition2(
    const Characteristic& characteristic, int start) const {
  if (num_bits_ == 2 && start == 0)
    return characteristic.GetContainerCondition2(container_pos_);
  else if (num_bits_ == 3 && start == 0)
    return characteristic.GetContainerCondition3(container_pos_).GetBits01();
  else if (num_bits_ == 3 && start == 1)
    return characteristic.GetContainerCondition3(container_pos_).GetBits12();
  assert(!"bit index error");
  return 0;
}

Condition<3> ContainerConditionProxy::GetCondition3(
    const Characteristic& characteristic, int start) const {
  if (num_bits_ == 3 && start == 0)
    return characteristic.GetContainerCondition3(container_pos_);
  assert(!"bit index error");
  return 0;
}

bool ContainerConditionProxy::SetCondition(Characteristic& characteristic,
                                           uint64_t value) const {
  // assert(GetCondition(characteristic));
  if (GetCondition(characteristic) == value) return true;
  characteristic.Touch(*this);
  if (num_bits_ == 1)
    return characteristic.SetContainerCondition1(container_pos_,
                                                 Condition<1>(value));
  else if (num_bits_ == 2)
    return characteristic.SetContainerCondition2(container_pos_,
                                                 Condition<2>(value));
  else if (num_bits_ == 3)
    return characteristic.SetContainerCondition3(container_pos_,
                                                 Condition<3>(value));
  assert(!"invalid number of bits");
  return 0;
}

bool ContainerConditionProxy::SetCondition1(Characteristic& characteristic,
                                            Condition<1> cond,
                                            int start) const {
  if (num_bits_ == 1 && start == 0) {
    const Condition<1> old_cond(
        characteristic.GetContainerCondition1(container_pos_));
    if (old_cond != cond) characteristic.Touch(*this);
    return characteristic.SetContainerCondition1(container_pos_, cond);
  } else if (num_bits_ == 2 && start == 0) {
    Condition<2> old_cond(
        characteristic.GetContainerCondition2(container_pos_));
    Condition<2> new_cond(old_cond);
    new_cond.MergeBit0(cond);
    if (old_cond != new_cond) characteristic.Touch(*this);
    return characteristic.SetContainerCondition2(container_pos_, new_cond);
  } else if (num_bits_ == 2 && start == 1) {
    Condition<2> old_cond(
        characteristic.GetContainerCondition2(container_pos_));
    Condition<2> new_cond(old_cond);
    new_cond.MergeBit1(cond);
    if (old_cond != new_cond) characteristic.Touch(*this);
    return characteristic.SetContainerCondition2(container_pos_, new_cond);
  } else if (num_bits_ == 3 && start == 0) {
    Condition<3> old_cond(
        characteristic.GetContainerCondition3(container_pos_));
    Condition<3> new_cond(old_cond);
    new_cond.MergeBit0(cond);
    if (old_cond != new_cond) characteristic.Touch(*this);
    return characteristic.SetContainerCondition3(container_pos_, new_cond);
  } else if (num_bits_ == 3 && start == 1) {
    Condition<3> old_cond(
        characteristic.GetContainerCondition3(container_pos_));
    Condition<3> new_cond(old_cond);
    new_cond.MergeBit1(cond);
    if (old_cond != new_cond) characteristic.Touch(*this);
    return characteristic.SetContainerCondition3(container_pos_, new_cond);
  } else if (num_bits_ == 3 && start == 2) {
    Condition<3> old_cond(
        characteristic.GetContainerCondition3(container_pos_));
    Condition<3> new_cond(old_cond);
    new_cond.MergeBit2(cond);
    if (old_cond != new_cond) characteristic.Touch(*this);
    return characteristic.SetContainerCondition3(container_pos_, new_cond);
  }
  assert(!"bit index error");
  return 0;
}

bool ContainerConditionProxy::SetCondition2(Characteristic& characteristic,
                                            Condition<2> cond,
                                            int start) const {
  if (num_bits_ == 2 && start == 0) {
    Condition<2> old_cond(
        characteristic.GetContainerCondition2(container_pos_));
    if (old_cond != cond) characteristic.Touch(*this);
    return characteristic.SetContainerCondition2(container_pos_, cond);
  } else if (num_bits_ == 3 && start == 0) {
    Condition<3> old_cond(
        characteristic.GetContainerCondition3(container_pos_));
    Condition<3> new_cond(old_cond);
    new_cond.MergeBits01(cond);
    if (old_cond != new_cond) characteristic.Touch(*this);
    return characteristic.SetContainerCondition3(container_pos_, new_cond);
  } else if (num_bits_ == 3 && start == 1) {
    Condition<3> old_cond(
        characteristic.GetContainerCondition3(container_pos_));
    Condition<3> new_cond(old_cond);
    new_cond.MergeBits12(cond);
    if (old_cond != new_cond) characteristic.Touch(*this);
    return characteristic.SetContainerCondition3(container_pos_, new_cond);
  }
  assert(!"bit index error");
  return 0;
}

bool ContainerConditionProxy::SetCondition3(Characteristic& characteristic,
                                            Condition<3> cond,
                                            int start) const {
  if (num_bits_ == 3 && start == 0) {
    Condition<3> old_cond(
        characteristic.GetContainerCondition3(container_pos_));
    if (old_cond != cond) characteristic.Touch(*this);
    return characteristic.SetContainerCondition3(container_pos_, cond);
  }
  assert(!"bit index error");
  return 0;
}

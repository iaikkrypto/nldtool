#include "virtual_condition_proxy.h"

#include "mask_condition_proxy.h"

VirtualConditionProxy::VirtualConditionProxy(int num_bits)
    : ConditionProxy(num_bits) {}

VirtualConditionProxy::~VirtualConditionProxy() {}

int VirtualConditionProxy::IsPosKonstant(int pos) const {
  for (int i = 0; i < condition_proxy_ptr_.size(); i++)
    if (condition_proxy_ptr_[i].start_vcp <= pos &&
        pos < condition_proxy_ptr_[i].start_vcp +
                  condition_proxy_ptr_[i].num_bits)
      return condition_proxy_ptr_[i].lcp->IsPosKonstant(pos);
  return -1;
}

const std::set<StepUpdate> VirtualConditionProxy::GetStepsToUpdate() const {
  std::set<StepUpdate> list;
  for (auto i : condition_proxy_ptr_)
    for (auto j : i.lcp->GetStepsToUpdate()) list.insert(j);
  return list;
}

void VirtualConditionProxy::AddStepToUpdate(StepUpdate step) {
  for (auto i : condition_proxy_ptr_) i.lcp->AddStepToUpdate(step);
}

void VirtualConditionProxy::AddConditionProxy(
    int num_bits, int start_vcp, std::shared_ptr<LinkableConditionProxy> lcp,
    int start_lcp) {
  assert(lcp.get());
  assert(num_bits > 0);
  assert(start_vcp + num_bits <= num_bits_);
  assert(start_lcp + num_bits <= lcp->GetNumBits());
  condition_proxy_ptr_.push_back({num_bits, start_vcp, lcp, start_lcp});
}

void VirtualConditionProxy::AddConditionProxy(
    int num_bits, int start_vcp, std::shared_ptr<VirtualConditionProxy> vcp) {
  assert(vcp.get());
  for (auto x : vcp->condition_proxy_ptr_) {
    int new_start_vcp = std::max(x.start_vcp, start_vcp);
    int new_end_vcp = std::min(x.start_vcp + x.num_bits, start_vcp + num_bits);
    int new_num_bits = new_end_vcp - new_start_vcp;
    int new_start_lcp = x.start_lcp + new_start_vcp - x.start_vcp;
    if (new_num_bits > 0)
      AddConditionProxy(new_num_bits, new_start_vcp, x.lcp, new_start_lcp);
  }
}

const std::vector<Bitpos> VirtualConditionProxy::GetBitposList() const {
  std::vector<Bitpos> list;
  for (auto x : condition_proxy_ptr_) {
    std::shared_ptr<MaskConditionProxy> smcp(
        std::dynamic_pointer_cast<MaskConditionProxy>(x.lcp));
    if (smcp) list.push_back(smcp->GetConditionMaskPos());
  }
  return list;
}

Condition<1> VirtualConditionProxy::GetVirtualCondition1(
    const Characteristic& characteristic) const {
  Condition<1> vbc1(0xf);
  for (int i = 0; i < condition_proxy_ptr_.size(); ++i) {
    const int num_bits = condition_proxy_ptr_[i].num_bits;
    const int start_virt = condition_proxy_ptr_[i].start_vcp;
    std::shared_ptr<LinkableConditionProxy> lcp = condition_proxy_ptr_[i].lcp;
    const int start_link = condition_proxy_ptr_[i].start_lcp;
    if (num_bits == 1) {
      const Condition<1> bc = lcp->GetCondition1(characteristic, start_link);
      if (start_virt == 0)
        vbc1.Filter(bc);
      else
        assert(!"bit index error");
    } else
      assert(!"bit index error");
  }
  return vbc1;
}

Condition<2> VirtualConditionProxy::GetVirtualCondition2(
    const Characteristic& characteristic) const {
  Condition<2> vbc2(0xffff);
  for (int i = 0; i < condition_proxy_ptr_.size(); ++i) {
    const int num_bits = condition_proxy_ptr_[i].num_bits;
    const int start_virt = condition_proxy_ptr_[i].start_vcp;
    std::shared_ptr<LinkableConditionProxy> lcp = condition_proxy_ptr_[i].lcp;
    const int start_link = condition_proxy_ptr_[i].start_lcp;
    if (num_bits == 1) {
      const Condition<1> bc = lcp->GetCondition1(characteristic, start_link);
      if (start_virt == 0)
        vbc2.MergeBit0(bc);
      else if (start_virt == 1)
        vbc2.MergeBit1(bc);
      else
        assert(!"bit index error");
    } else if (num_bits == 2) {
      const Condition<2> bc = lcp->GetCondition2(characteristic, start_link);
      if (start_virt == 0)
        vbc2.Filter(bc);
      else
        assert(!"bit index error");
    } else
      assert(!"bit index error");
  }
  return vbc2;
}

Condition<3> VirtualConditionProxy::GetVirtualCondition3(
    const Characteristic& characteristic) const {
  Condition<3> vbc3(~0ull);
  for (int i = 0; i < condition_proxy_ptr_.size(); ++i) {
    const int num_bits = condition_proxy_ptr_[i].num_bits;
    const int start_virt = condition_proxy_ptr_[i].start_vcp;
    std::shared_ptr<LinkableConditionProxy> lcp = condition_proxy_ptr_[i].lcp;
    const int start_link = condition_proxy_ptr_[i].start_lcp;
    if (num_bits == 1) {
      const Condition<1> bc = lcp->GetCondition1(characteristic, start_link);
      if (start_virt == 0)
        vbc3.MergeBit0(bc);
      else if (start_virt == 1)
        vbc3.MergeBit1(bc);
      else if (start_virt == 2)
        vbc3.MergeBit2(bc);
      else
        assert(!"bit index error");
    } else if (num_bits == 2) {
      const Condition<2> bc = lcp->GetCondition2(characteristic, start_link);
      if (start_virt == 0)
        vbc3.MergeBits01(bc);
      else if (start_virt == 1)
        vbc3.MergeBits12(bc);
      else
        assert(!"bit index error");
    } else if (num_bits == 3) {
      const Condition<3> bc = lcp->GetCondition3(characteristic, start_link);
      if (start_virt == 0)
        vbc3.Filter(bc);
      else
        assert(!"bit index error");
    } else
      assert(!"bit index error");
  }
  return vbc3;
}

bool VirtualConditionProxy::SetVirtualCondition1(Characteristic& characteristic,
                                                 Condition<1> vbc1) const {
  //  Condition<1> vbc1(0xf);
  bool result = true;
  for (int i = 0; i < condition_proxy_ptr_.size(); ++i) {
    const int num_bits = condition_proxy_ptr_[i].num_bits;
    const int start_virt = condition_proxy_ptr_[i].start_vcp;
    std::shared_ptr<LinkableConditionProxy> lcp = condition_proxy_ptr_[i].lcp;
    const int start_link = condition_proxy_ptr_[i].start_lcp;
    if (num_bits == 1) {
      Condition<1> bc;
      if (start_virt == 0)
        bc = vbc1;
      else
        assert(!"bit index error");
      result &= lcp->SetCondition1(characteristic, bc, start_link);
    } else
      assert(!"bit index error");
  }
  return result;
}

bool VirtualConditionProxy::SetVirtualCondition2(Characteristic& characteristic,
                                                 Condition<2> vbc2) const {
  bool result = true;
  for (int i = 0; i < condition_proxy_ptr_.size(); ++i) {
    const int num_bits = condition_proxy_ptr_[i].num_bits;
    const int start_virt = condition_proxy_ptr_[i].start_vcp;
    std::shared_ptr<LinkableConditionProxy> lcp = condition_proxy_ptr_[i].lcp;
    const int start_link = condition_proxy_ptr_[i].start_lcp;
    if (num_bits == 1) {
      Condition<1> bc;
      if (start_virt == 0)
        bc = vbc2.GetBit0();
      else if (start_virt == 1)
        bc = vbc2.GetBit1();
      else
        assert(!"bit index error");
      result &= lcp->SetCondition1(characteristic, bc, start_link);
    } else if (num_bits == 2) {
      Condition<2> bc;
      if (start_virt == 0)
        bc = vbc2;
      else
        assert(!"bit index error");
      result &= lcp->SetCondition2(characteristic, bc, start_link);
    } else
      assert(!"bit index error");
  }
  return result;
}

bool VirtualConditionProxy::SetVirtualCondition3(Characteristic& characteristic,
                                                 Condition<3> vbc3) const {
  bool result = true;
  for (int i = 0; i < condition_proxy_ptr_.size(); ++i) {
    const int num_bits = condition_proxy_ptr_[i].num_bits;
    const int start_virt = condition_proxy_ptr_[i].start_vcp;
    std::shared_ptr<LinkableConditionProxy> lcp = condition_proxy_ptr_[i].lcp;
    const int start_link = condition_proxy_ptr_[i].start_lcp;
    if (num_bits == 1) {
      Condition<1> bc;
      if (start_virt == 0)
        bc = vbc3.GetBit0();
      else if (start_virt == 1)
        bc = vbc3.GetBit1();
      else if (start_virt == 2)
        bc = vbc3.GetBit2();
      else
        assert(!"bit index error");
      result = lcp->SetCondition1(characteristic, bc, start_link);
    } else if (num_bits == 2) {
      Condition<2> bc;
      if (start_virt == 0)
        bc = vbc3.GetBits01();
      else if (start_virt == 1)
        bc = vbc3.GetBits12();
      else
        assert(!"bit index error");
      result = lcp->SetCondition2(characteristic, bc, start_link);
    } else if (num_bits == 3) {
      Condition<3> bc;
      if (start_virt == 0)
        bc = vbc3;
      else
        assert(!"bit index error");
      result = lcp->SetCondition3(characteristic, bc, start_link);
    } else
      assert(!"bit index error");
  }
  return result;
}

Condition<1> VirtualConditionProxy::GetCondition1(
    const Characteristic& characteristic, int start) const {
  if (num_bits_ == 1 && start == 0)
    return GetVirtualCondition1(characteristic);
  else if (num_bits_ == 2 && start == 0)
    return GetVirtualCondition2(characteristic).GetBit0();
  else if (num_bits_ == 2 && start == 1)
    return GetVirtualCondition2(characteristic).GetBit1();
  else if (num_bits_ == 3 && start == 0)
    return GetVirtualCondition3(characteristic).GetBit0();
  else if (num_bits_ == 3 && start == 1)
    return GetVirtualCondition3(characteristic).GetBit1();
  else if (num_bits_ == 3 && start == 2)
    return GetVirtualCondition3(characteristic).GetBit2();
  else
    return condition_proxy_ptr_[start].lcp->GetCondition1(characteristic);
  // assert(!"bit index error");
  return Condition<1>("#");
}

Condition<2> VirtualConditionProxy::GetCondition2(
    const Characteristic& characteristic, int start) const {
  if (num_bits_ == 2 && start == 0)
    return GetVirtualCondition2(characteristic);
  else if (num_bits_ == 3 && start == 0)
    return GetVirtualCondition3(characteristic).GetBits01();
  else if (num_bits_ == 3 && start == 1)
    return GetVirtualCondition3(characteristic).GetBits12();
  else
    assert(!"bit index error");
  return 0;
}

Condition<3> VirtualConditionProxy::GetCondition3(
    const Characteristic& characteristic, int start) const {
  if (num_bits_ == 3 && start == 0)
    return GetVirtualCondition3(characteristic);
  else
    assert(!"bit index error");
  return 0;
}

// TODO: merge vs. set
bool VirtualConditionProxy::SetCondition1(Characteristic& characteristic,
                                          Condition<1> condition,
                                          int start) const {
  if (num_bits_ == 1 && start == 0)
    return SetVirtualCondition1(characteristic, condition);
  else if (num_bits_ == 2 && start == 0) {
    Condition<2> bc = GetVirtualCondition2(characteristic);
    bc.MergeBit0(condition);
    return SetVirtualCondition2(characteristic, bc);
  } else if (num_bits_ == 2 && start == 1) {
    Condition<2> bc = GetVirtualCondition2(characteristic);
    bc.MergeBit1(condition);
    return SetVirtualCondition2(characteristic, bc);
  } else if (num_bits_ == 3 && start == 0) {
    Condition<3> bc = GetVirtualCondition3(characteristic);
    bc.MergeBit0(condition);
    return SetVirtualCondition3(characteristic, bc);
  } else if (num_bits_ == 3 && start == 1) {
    Condition<3> bc = GetVirtualCondition3(characteristic);
    bc.MergeBit1(condition);
    return SetVirtualCondition3(characteristic, bc);
  } else if (num_bits_ == 3 && start == 2) {
    Condition<3> bc = GetVirtualCondition3(characteristic);
    bc.MergeBit2(condition);
    return SetVirtualCondition3(characteristic, bc);
  } else
    assert(!"bit index error");
  return 0;
}

// TODO: merge vs. set
bool VirtualConditionProxy::SetCondition2(Characteristic& characteristic,
                                          Condition<2> condition,
                                          int start) const {
  if (num_bits_ == 2 && start == 0)
    return SetVirtualCondition2(characteristic, condition);
  else if (num_bits_ == 3 && start == 0) {
    Condition<3> bc = GetVirtualCondition3(characteristic);
    bc.MergeBits01(condition);
    return SetVirtualCondition3(characteristic, bc);
  } else if (num_bits_ == 3 && start == 1) {
    Condition<3> bc = GetVirtualCondition3(characteristic);
    bc.MergeBits12(condition);
    return SetVirtualCondition3(characteristic, bc);
  } else
    assert(!"bit index error");
  return 0;
}

// TODO: merge vs. set
bool VirtualConditionProxy::SetCondition3(Characteristic& characteristic,
                                          Condition<3> condition,
                                          int start) const {
  if (num_bits_ == 3 && start == 0)
    return SetVirtualCondition3(characteristic, condition);
  else
    assert(!"bit index error");
  return 0;
}

uint64_t VirtualConditionProxy::GetCondition(
    const Characteristic& characteristic) const {
  if (num_bits_ == 1)
    return GetVirtualCondition1(characteristic);
  else if (num_bits_ == 2)
    return GetVirtualCondition2(characteristic);
  else if (num_bits_ == 3)
    return GetVirtualCondition3(characteristic);
  else
    assert(!"bit index error");
  return 0;
}

bool VirtualConditionProxy::SetCondition(Characteristic& characteristic,
                                         uint64_t value) const {
  if (num_bits_ == 1)
    return SetVirtualCondition1(characteristic, Condition<1>(value));
  else if (num_bits_ == 2)
    return SetVirtualCondition2(characteristic, Condition<2>(value));
  else if (num_bits_ == 3)
    return SetVirtualCondition3(characteristic, Condition<3>(value));
  else
    assert(!"bit index error");
  return 0;
}

int VirtualConditionProxy::GetHammingWeight(
    const Characteristic& characteristic) const {
  if (num_bits_ <= 3)
    return nldtool::HammingWeight(GetCondition(characteristic));
  int hw = 0;
  for (auto x : condition_proxy_ptr_)
    hw += x.lcp->GetHammingWeight(characteristic);
  return hw;
}

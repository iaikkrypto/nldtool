#include "mask_condition_proxy.h"

MaskConditionProxy::MaskConditionProxy(int num_bits, Bitpos condition_mask_pos)
    : ConditionProxy(num_bits),
      VirtualConditionProxy(num_bits),
      condition_mask_pos_(condition_mask_pos) {}

MaskConditionProxy::~MaskConditionProxy() {}

bool MaskConditionProxy::IsMaskConditionProxy() const { return true; }

Bitpos MaskConditionProxy::GetConditionMaskPos() const {
  return condition_mask_pos_;
}

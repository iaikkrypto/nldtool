#ifndef MASK_CONDITION_PROXY_H_
#define MASK_CONDITION_PROXY_H_

#include "bitpos.h"
#include "virtual_condition_proxy.h"

class MaskConditionProxy : public VirtualConditionProxy {
 public:
  MaskConditionProxy(int num_bits, Bitpos condition_mask_pos);
  virtual ~MaskConditionProxy();
  virtual bool IsMaskConditionProxy() const;
  virtual Bitpos GetConditionMaskPos() const;

 protected:
  Bitpos condition_mask_pos_;
};

#endif  // MASK_CONDITION_PROXY_H_

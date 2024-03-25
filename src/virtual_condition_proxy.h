#ifndef VIRTUAL_CONDITION_PROXY_H_
#define VIRTUAL_CONDITION_PROXY_H_

#include <set>
#include <vector>

#include "bitpos.h"
#include "characteristic.h"
#include "condition.h"
#include "condition_proxy.h"
#include "linkable_condition_proxy.h"
#include "step_update.h"

class VirtualConditionProxy : public virtual ConditionProxy {
 public:
  VirtualConditionProxy(int num_bits);
  virtual ~VirtualConditionProxy();
  virtual void AddConditionProxy(int num_bits, int start_vcp,
                                 std::shared_ptr<LinkableConditionProxy> lcp,
                                 int start_lcp);
  virtual void AddConditionProxy(int num_bits, int start_vcp,
                                 std::shared_ptr<VirtualConditionProxy> vcp);
  virtual uint64_t GetCondition(const Characteristic& characteristic) const;
  virtual bool SetCondition(Characteristic& characteristic,
                            uint64_t value) const;
  virtual Condition<1> GetVirtualCondition1(
      const Characteristic& characteristic) const;
  virtual Condition<2> GetVirtualCondition2(
      const Characteristic& characteristic) const;
  virtual Condition<3> GetVirtualCondition3(
      const Characteristic& characteristic) const;
  virtual bool SetVirtualCondition1(Characteristic& characteristic,
                                    Condition<1> condition) const;
  virtual bool SetVirtualCondition2(Characteristic& characteristic,
                                    Condition<2> condition) const;
  virtual bool SetVirtualCondition3(Characteristic& characteristic,
                                    Condition<3> condition) const;
  virtual Condition<1> GetCondition1(const Characteristic& characteristic,
                                     int start = 0) const;
  virtual Condition<2> GetCondition2(const Characteristic& characteristic,
                                     int start = 0) const;
  virtual Condition<3> GetCondition3(const Characteristic& characteristic,
                                     int start = 0) const;
  virtual bool SetCondition1(Characteristic& characteristic,
                             Condition<1> condition, int start = 0) const;
  virtual bool SetCondition2(Characteristic& characteristic,
                             Condition<2> condition, int start = 0) const;
  virtual bool SetCondition3(Characteristic& characteristic,
                             Condition<3> condition, int start = 0) const;
  virtual int IsPosKonstant(int pos) const;
  virtual const std::set<StepUpdate> GetStepsToUpdate() const;
  virtual void AddStepToUpdate(StepUpdate step);
  virtual const std::vector<Bitpos> GetBitposList() const;
  virtual int GetHammingWeight(const Characteristic& characteristic) const;

 protected:
  struct ConditionProxyPointer {
    int num_bits;
    int start_vcp;
    std::shared_ptr<LinkableConditionProxy> lcp;
    int start_lcp;
  };
  std::vector<ConditionProxyPointer> condition_proxy_ptr_;
};

#endif  // VIRTUAL_CONDITION_PROXY_H_

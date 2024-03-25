#ifndef CONTAINER_CONDITION_PROXY_H_
#define CONTAINER_CONDITION_PROXY_H_

#include <memory>
#include <set>
#include <vector>

#include "bitpos.h"
#include "condition.h"
#include "condition_proxy.h"
#include "linkable_condition_proxy.h"
#include "mask_condition_proxy.h"
#include "step_update.h"

class ConditionProxy;
class KonstantConditionProxy;
class VirtualConditionProxy;
class ContainerConditionProxy;

typedef std::shared_ptr<ConditionProxy> ConditionProxyPtr;

class Characteristic;

class ContainerConditionProxy : public MaskConditionProxy,
                                public LinkableConditionProxy {
 public:
  ContainerConditionProxy(int num_bits, Bitpos condition_mask_pos,
                          Bitpos container_pos);
  virtual ~ContainerConditionProxy();
  virtual uint64_t GetCondition(const Characteristic& characteristic) const;
  virtual bool SetCondition(Characteristic& characteristic,
                            uint64_t value) const;
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
  virtual void AddStepToUpdate(StepUpdate step);
  virtual const std::set<StepUpdate> GetStepsToUpdate() const;
  virtual Bitpos GetContainerPos() const;
  virtual std::string ToString() const;

 protected:
  Bitpos container_pos_;
  std::set<StepUpdate> step_update_list_;
};

#endif  // CONTAINER_CONDITION_PROXY_H_

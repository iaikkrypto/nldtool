#ifndef CONDITION_PROXY_H_
#define CONDITION_PROXY_H_

#include <memory>
#include <set>
#include <vector>

#include "bitpos.h"
#include "condition.h"
#include "step_update.h"

class Characteristic;

class ConditionProxy {
 public:
  ConditionProxy(int num_bits = 0);
  virtual ~ConditionProxy();
  virtual uint64_t GetCondition(const Characteristic& characteristic) const = 0;
  virtual bool SetCondition(Characteristic& characteristic,
                            uint64_t value) const = 0;
  virtual int GetHammingWeight(const Characteristic& characteristic) const;
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
  virtual void AddStepToUpdate(StepUpdate step);
  virtual const std::set<StepUpdate> GetStepsToUpdate() const;
  virtual const std::vector<Bitpos> GetBitposList() const;
  virtual std::string ToString() const;
  virtual int GetNumBits() const;
  virtual int IsPosKonstant(int pos) const;
  virtual bool IsMaskConditionProxy() const;
  virtual Bitpos GetConditionMaskPos() const;
  friend std::ostream& operator<<(std::ostream& os,
                                  ConditionProxy& condition_proxy);

 protected:
  int num_bits_;
};

typedef std::shared_ptr<ConditionProxy> ConditionProxyPtr;

#endif  // CONDITION_PROXY_H_

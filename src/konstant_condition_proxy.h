#ifndef KONSTANT_CONDITION_PROXY_H_
#define KONSTANT_CONDITION_PROXY_H_

#include <cstdint>
#include <string>

#include "bitpos.h"
#include "characteristic.h"
#include "linkable_condition_proxy.h"

class KonstantConditionProxy : public LinkableConditionProxy {
 public:
  KonstantConditionProxy(int num_bits, uint64_t condition = 1);
  virtual ~KonstantConditionProxy();
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
  virtual std::string ToString() const;

 protected:
  const uint64_t condition_;
};

#endif  // KONSTANT_CONDITION_PROXY_H_

#ifndef LINKABLE_CONDITION_PROXY_H_
#define LINKABLE_CONDITION_PROXY_H_

#include "condition_proxy.h"

class LinkableConditionProxy : public virtual ConditionProxy {
 public:
  LinkableConditionProxy() {}
  virtual ~LinkableConditionProxy() {}
};

#endif  // LINKABLE_CONDITION_PROXY_H_

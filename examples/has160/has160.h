#ifndef HAS160_H_
#define HAS160_H_

#include "crypto.h"
#include "functions.h"

class Has160 : public Crypto {
 public:
  static void AddToOptions(cxxopts::Options& options);
  Has160(cxxopts::Options& options);

 private:
  ConditionWordPtr W[80];
  ConditionWordPtr tA[85];
  ConditionWordPtr* A;
  ConditionWordPtr F_[80];
  int num_rounds_;
};

#endif  // HAS160_H_

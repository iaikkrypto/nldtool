#ifndef RIPEMD128_H_
#define RIPEMD128_H_

#include "crypto.h"

class Ripemd128 : public Crypto {
 public:
  static const int H;

  static void AddToOptions(cxxopts::Options& options);
  Ripemd128(cxxopts::Options& options);

  static const uint32_t KA[4];
  static const uint32_t KB[4];
  static const int SA[64];
  static const int SB[64];
  static const int XA[64];
  static const int XB[64];

 protected:
  int num_rounds_;
  ConditionWordPtr W[64];
  ConditionWordPtr tA[64 + 4 + 4];
  ConditionWordPtr* A;
  ConditionWordPtr tB[64 + 4];
  ConditionWordPtr* B;
  ConditionWordPtr F[64];
  ConditionWordPtr G[64];
};

#endif  // RIPEMD128_H_

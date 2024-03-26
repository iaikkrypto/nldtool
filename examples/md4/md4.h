#ifndef MD4_H_
#define MD4_H_

#include "crypto.h"

/*! \class Md4
 *  \brief Implementation of the MD4 crypto function.
 *
 *  https://tools.ietf.org/html/rfc1320
 */
class Md4 : public Crypto {
 public:
  static const uint32_t K[3];
  static const int S[12];
  static const int P[48];

  static void AddToOptions(cxxopts::Options& options);
  Md4(cxxopts::Options& options);

 protected:
  int num_rounds_;
  ConditionWordPtr W[16];
  ConditionWordPtr tA[48 + 4];
  ConditionWordPtr* A = &tA[4];
  ConditionWordPtr F[48];
};

#endif  // MD4_H_

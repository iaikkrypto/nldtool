#ifndef MD5_H_
#define MD5_H_

#include "crypto.h"

/*! \class Md5
 *  \brief Implementation of the MD5 crypto function.
 *
 *  https://tools.ietf.org/html/rfc1321
 */
class Md5 : public Crypto {
 public:
  class ADD4ADD2;
  static const int H;
  static const int L;
  static const uint32_t K[64];
  static const int S[64];

  static void AddToOptions(cxxopts::Options& options);
  Md5(cxxopts::Options& options);

 protected:
  int num_rounds_;
  ConditionWordPtr W[64];
  ConditionWordPtr tA[64 + 4];
  ConditionWordPtr* A;
  ConditionWordPtr F[64];
  ConditionWordPtr B[64];
};

#endif  // MD5_H_

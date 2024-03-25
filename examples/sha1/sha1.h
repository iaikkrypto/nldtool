#ifndef SHA1_H_
#define SHA1_H_

#include "crypto.h"
#include "cxxopts.hpp"

/*! \class Sha1
 *  \brief Implementation of the SHA-1 crypto function.
 *
 *  https://tools.ietf.org/html/rfc3174
 *  http://csrc.nist.gov/publications/fips/fips180-4/fips-180-4.pdf
 */
class Sha1 : public Crypto {
 public:
  static void AddToOptions(cxxopts::Options& options);
  Sha1(cxxopts::Options& options);
  class MEXP;
  class IFADD5;
  class XOR3ADD5;
  class MAJADD5;

 protected:
  int num_rounds_;
  ConditionWordPtr W[80];
  ConditionWordPtr F[80];
  ConditionWordPtr tA[80 + 10];
  ConditionWordPtr* A_;
};

#endif  // SHA1_H_

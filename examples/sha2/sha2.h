#ifndef SHA2_H_
#define SHA2_H_

#include "crypto.h"
#include "cxxopts.hpp"

/*! \class Sha2
 *  \brief Implementation of the family of SHA-2 crypto functions.
 *
 *  https://tools.ietf.org/html/rfc4634
 *  http://csrc.nist.gov/publications/fips/fips180-4/fips-180-4.pdf
 */
class Sha2 : public Crypto {
 public:
  static const uint64_t K512[80];
  static constexpr int Rot256[12] = {7, 18, 3,  17, 19, 10,
                                     6, 11, 25, 2,  13, 22};
  static constexpr int Rot512[12] = {1,  8,  7,  19, 61, 6,
                                     14, 18, 41, 28, 34, 39};

  static const int H;
  static const int L;
  static const int F;

  template <int R0, int R1, int S2>
  class sigma;
  template <int R0, int R1, int S2>
  class Sigma;
  class SADD;

  static void AddToOptions(cxxopts::Options& options);
  Sha2(cxxopts::Options& options);
  void InitWords();
  void InitSub();

 protected:
  const int* S;
  int num_rounds_;
  int use_linear_layer_;
  int s, h, v, V, m;

  ConditionWordPtr W[80];
  ConditionWordPtr tE[80 + 8];
  ConditionWordPtr tA[80 + 8];
  ConditionWordPtr* E = tE + H;
  ConditionWordPtr* A = tA + H;
  ConditionWordPtr ms0[80];
  ConditionWordPtr ms1[80];
  ConditionWordPtr mS0[80];
  ConditionWordPtr mS1[80];
  ConditionWordPtr mIF[80];
  ConditionWordPtr mMJ[80];
  ConditionWordPtr KW[80];
  ConditionWordPtr CW[80];
  ConditionWordPtr CE[80];
  ConditionWordPtr CA[80];
};

#endif  // SHA2_H_

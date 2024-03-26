#ifndef RIPEMD160_H_
#define RIPEMD160_H_

#include "crypto.h"

/*! \class Ripemd160
 *  \brief Implementation of the crypto function RIPEMD-160
 *
 *  https://homes.esat.kuleuven.be/~bosselae/ripemd160.html
 */
class Ripemd160 : public Crypto {
 public:
  class ADD4ADD2;
  class XOR3ADD4ADD2;
  class IFADD4ADD2;
  class ONXADD4ADD2;
  static const int H;
  static const int L;
  static const int S;

  static void AddToOptions(cxxopts::Options& options);
  Ripemd160(cxxopts::Options& options);

  static const uint32_t KA[5];
  static const uint32_t KB[5];
  static const int SA[80];
  static const int SB[80];
  static const int XA[80];
  static const int XB[80];

 protected:
  int num_rounds_;
  int start_round_;
  ConditionWordPtr K;
  ConditionWordPtr W[80];
  ConditionWordPtr tA[80 + 5 + 5];
  ConditionWordPtr tB[80 + 5];
  ConditionWordPtr* A;
  ConditionWordPtr* B;
  ConditionWordPtr D[80];
  ConditionWordPtr E[80];
  ConditionWordPtr F[80];
  ConditionWordPtr G[80];
  ConditionWordPtr CA[80];
  ConditionWordPtr CB[80];
  ConditionWordPtr CD[80];
  ConditionWordPtr CE[80];
  ConditionWordPtr CAo[80];
  ConditionWordPtr CAi[80];
  ConditionWordPtr CBo[80];
  ConditionWordPtr CBi[80];
};

#endif  // RIPEMD160_H_

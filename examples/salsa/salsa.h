#ifndef SALSA_H_
#define SALSA_H_

#include "crypto.h"

/*! \class Salsa
 *  \brief Implementation of the Salsa20 stream cipher.
 */
class Salsa : public Crypto {
 public:
  static void AddToOptions(cxxopts::Options& options);
  Salsa(cxxopts::Options& options);

 protected:
  int num_rounds_;
  ConditionWordPtr A[21][16];
  ConditionWordPtr B[21][16];

  ConditionWordPtr A2[21][16];
  ConditionWordPtr B2[21][16];
};

#endif  // SALSA_H_

#ifndef DHASUB_H_
#define DHASUB_H_

#include "crypto.h"
#include "cxxopts.hpp"

/*! \class Dha256
 *  \brief Implementation of the DHA256 crypto function
 *
 *  http://csrc.nist.gov/groups/ST/crypto/documents/ChangD_DHA256.pdf
 */
class Dha256 : public Crypto {
 public:
  static const uint64_t K256[64];
  static const int I = 4;
  static const int L = 3;

  static void AddToOptions(cxxopts::Options& options);
  Dha256(cxxopts::Options& options);
  bool Callback(std::string function, Characteristic& characteristic,
                std::mt19937& rng, Logfile& logfile);

 protected:
  int num_rounds_;
  ConditionWordPtr W_[64];
  ConditionWordPtr S0_[64];
  ConditionWordPtr S1_[64];
  ConditionWordPtr tE_[64 + 8];
  ConditionWordPtr tA_[64 + 8];
  ConditionWordPtr* E_;
  ConditionWordPtr* A_;
  ConditionWordPtr F0_[64];
  ConditionWordPtr F1_[64];
  ConditionWordPtr SS0_[64];
  ConditionWordPtr SS1_[64];

  class P0;
  class P1;

  class PP0;
  class PP1;
};

#endif  // DHASUB_H_

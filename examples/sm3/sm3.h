#ifndef SM3SUB_H_
#define SM3SUB_H_

#include "crypto.h"

class Sm3 : public Crypto {
 public:
  static const uint64_t K256[64];
  static const int I;
  static const int L;

  static void AddToOptions(cxxopts::Options& options);
  Sm3(cxxopts::Options& options);

  bool Callback(std::string function, Characteristic& characteristic,
                std::mt19937& rng, Logfile& logfile);
  bool FirstBlock(Characteristic& characteristic, std::mt19937& rng,
                  Logfile& logfile);

  void trivialBitSlice(int round);
  void trivialTwoBitSlice(int round);
  void oneRound4InTBS(int round);
  void oneRound4In(int round);
  void oneRound4InSp(int round);

 protected:
  int num_rounds_;
  int bitslice_flags_;
  ConditionWordPtr W_[68];
  ConditionWordPtr tE_[64 + 8];
  ConditionWordPtr tA_[64 + 8];
  ConditionWordPtr* E_;
  ConditionWordPtr* A_;
  ConditionWordPtr F_[64];
  ConditionWordPtr G_[64];
  ConditionWordPtr T_[64];
  ConditionWordPtr TK_[64];
  ConditionWordPtr TA_[64];
  ConditionWordPtr FD_[64];
  ConditionWordPtr FDTA_[64];
  ConditionWordPtr GH_[64];
  ConditionWordPtr GHT_[64];
  ConditionWordPtr P_[64];
  ConditionWordPtr S0_[68];
  ConditionWordPtr P1_[68];
  ConditionWordPtr Wp_[64];

  ConditionWordPtr W2_[68];
  ConditionWordPtr tE2_[64 + 8];
  ConditionWordPtr tA2_[64 + 8];
  ConditionWordPtr* E2_;
  ConditionWordPtr* A2_;
  ConditionWordPtr F2_[64];
  ConditionWordPtr G2_[64];
  ConditionWordPtr T2_[64];
  ConditionWordPtr TK2_[64];
  ConditionWordPtr TA2_[64];
  ConditionWordPtr FD2_[64];
  ConditionWordPtr FDTA2_[64];
  ConditionWordPtr GH2_[64];
  ConditionWordPtr GHT2_[64];
  ConditionWordPtr P2_[64];
  ConditionWordPtr S02_[68];
  ConditionWordPtr P12_[68];
  ConditionWordPtr Wp2_[64];

  ConditionWordPtr CAo_[64];
  ConditionWordPtr CPo_[64];
  ConditionWordPtr CAi_[64];
  ConditionWordPtr CPi_[64];
  ConditionWordPtr CA_[64];
  ConditionWordPtr CP_[64];
  ConditionWordPtr CT_[64];
  ConditionWordPtr CTA_[64];
  ConditionWordPtr CTP_[64];
  ConditionWordPtr CTA6_[64];
  ConditionWordPtr CTP6_[64];

  class P0;
  class P1;
  class IFB;
  class MAJB;
  class ADD3XOR2;
  class ADD3XOR2SP;
  class ADD2XOR2ADD2SP;
  class XOR2ADD3SP;
  class ADD4SP;
};

#endif  // SM3SUB_H_

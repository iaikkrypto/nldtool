#ifndef ICEPOLE_H_
#define ICEPOLE_H_

#include "crypto.h"

#define ICEPOLE_MAXROUNDS 12

class Icepole : public Crypto {
 public:
  static void AddToOptions(cxxopts::Options& options);
  Icepole(cxxopts::Options& options);
  static constexpr uint64_t ROUND_CONSTANTS[12] = {
      0x0091A2B3C4D5E6F7ULL, 0x0048D159E26AF37BULL, 0x002468ACF13579BDULL,
      0x00123456F89ABCDEULL, 0x00091A2BFC4D5E6FULL, 0x00048D15FE26AF37ULL,
      0x0002468AFF13579BULL, 0x000123457F89ABCDULL, 0x000091A2BFC4D5E6ULL,
      0x000048D1DFE26AF3ULL, 0x00002468EFF13579ULL, 0x00001234F7F89ABC};
  static constexpr int ROTATION_VALUES[4][5] = {{0, 36, 3, 41, 18},
                                                {1, 44, 10, 45, 2},
                                                {62, 6, 43, 15, 61},
                                                {28, 55, 25, 21, 56}};
  class SBOX;
  class isActive;
  class colActive;
  class Linear_Layer;

 protected:
  ConditionWordPtr S[ICEPOLE_MAXROUNDS + 1][4][5];
  ConditionWordPtr A[ICEPOLE_MAXROUNDS][4][5];
  ConditionWordPtr B[ICEPOLE_MAXROUNDS][4][5];
  ConditionWordPtr C[ICEPOLE_MAXROUNDS];
  ConditionWordPtr ACTIVE[ICEPOLE_MAXROUNDS][4];
  ConditionWordPtr C_ACTIVE[ICEPOLE_MAXROUNDS];

  int num_rounds_;
  int part_;
  int linear_flags_;
  float best_prob_;
  uint64_t best_char_;

  void mue_rho_lin(int rb);
  void mue_rho(int rb);
  void pi(int rb);
  void psi(int rb);
  void kappa(int rb);

  virtual bool Callback(std::string function, Characteristic& characteristic,
                        std::mt19937& rng, Logfile& logfile);
};

#endif  // ICEPOLE_H_

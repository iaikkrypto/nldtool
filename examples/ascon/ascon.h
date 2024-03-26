#ifndef ASCON_H_
#define ASCON_H_

#include "crypto.h"

/*! \class Ascon
 *  \brief Implementation of the Ascon authenticated encryption algorithm.
 *
 *  Implemented according to http://ascon.iaik.tugraz.at/specification.html
 */
class Ascon : public Crypto {
 public:
  static constexpr int RotVal[10] = {19, 28, 61, 39, 1, 6, 10, 17, 7, 41};

  static constexpr uint64_t RoundConstant[12] = {
      0x000000000000000000f0ULL, 0x000000000000000000e1ULL,
      0x000000000000000000d2ULL, 0x000000000000000000c3ULL,
      0x000000000000000000b4ULL, 0x000000000000000000a5ULL,
      0x00000000000000000096ULL, 0x00000000000000000087ULL,
      0x00000000000000000078ULL, 0x00000000000000000069ULL,
      0x0000000000000000005aULL, 0x0000000000000000004bULL};

  static constexpr uint8_t LUT[32] = {4,  11, 31, 20, 26, 21, 9,  2,  27, 5, 8,
                                      18, 29, 3,  6,  28, 30, 19, 7,  14, 0, 13,
                                      17, 24, 16, 12, 1,  25, 22, 10, 15, 23};

  template <int R0, int R1, int R2>
  class Sigma;
  class Sbox;
  class isActive;

  static void AddToOptions(cxxopts::Options& options);
  Ascon(cxxopts::Options& options);

  bool Callback(std::string function, Characteristic& characteristic,
                std::mt19937& rng, Logfile& logfile);

 protected:
  int num_rounds_;
  int start_round_;
  uint64_t best_char_;
  ConditionWordPtr S[40];
  ConditionWordPtr A[40][5];
  ConditionWordPtr T_A2[40];
  ConditionWordPtr B[40][5];
};

#endif  // ASCON_H_

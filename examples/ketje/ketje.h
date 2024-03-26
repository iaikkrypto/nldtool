#ifndef KETJE_H_
#define KETJE_H_

#include "crypto.h"

class Characteristic;

/*! \class Ketje
 *  \brief Reduced implementation of the authenticated encryption algorithm
 * Ketje.
 *
 *  http://ketje.noekeon.org/
 */
class Ketje : public Crypto {
 public:
  static constexpr uint64_t RC[24] = {
      0x0000000000000001, 0x0000000000008082, 0x800000000000808A,
      0x8000000080008000, 0x000000000000808B, 0x0000000080000001,
      0x8000000080008081, 0x8000000000008009, 0x000000000000008A,
      0x0000000000000088, 0x0000000080008009, 0x000000008000000A,
      0x000000008000808B, 0x800000000000008B, 0x8000000000008089,
      0x8000000000008003, 0x8000000000008002, 0x8000000000000080,
      0x000000000000800A, 0x800000008000000A, 0x8000000080008081,
      0x8000000000008080, 0x0000000080000001, 0x8000000080008008};
  static constexpr int R[5][5] = {{0, 36, 3, 41, 18},
                                  {1, 44, 10, 45, 2},
                                  {62, 6, 43, 15, 61},
                                  {28, 55, 25, 21, 56},
                                  {27, 20, 39, 8, 14}};
  static constexpr int pstar[25] = {0,  6,  12, 18, 24, 3,  9, 10, 16,
                                    22, 1,  7,  13, 19, 20, 4, 5,  11,
                                    17, 23, 2,  8,  14, 15, 21};
  class CHIRC;
  class LinearThetaRhoPi;
  class ID;
  class isActive;

  static void AddToOptions(cxxopts::Options& options);
  Ketje(cxxopts::Options& options);
  void InitWords();
  std::string GetName(char a, int i, int j);
  virtual bool Callback(std::string function, Characteristic& characteristic,
                        std::mt19937& rng, Logfile& logfile);

 protected:
  int num_rounds_;
  int rate_;
  uint64_t best_char_;
  ConditionWordPtr A[50 + 1][5][5];
  ConditionWordPtr B[50][5][5];
  ConditionWordPtr C[50][5][5];
  ConditionWordPtr S[50][5];
  ConditionWordPtr K[24];
  ConditionWordPtr K0;
};

#endif  // KETJE_H_

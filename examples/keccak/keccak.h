#ifndef KECCAK_H_
#define KECCAK_H_

#include "crypto.h"

class Characteristic;

/*! \class Keccak
 *  \brief Implementation of the Keccak crypto function
 *
 *  Winner of the SHA-3 competition. http://keccak.noekeon.org/
 */
class Keccak : public Crypto {
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

  class CHIRC;
  class LinearThetaRhoPi;

  static void AddToOptions(cxxopts::Options& options);
  Keccak(cxxopts::Options& options);
  void InitWords();
  std::string GetName(char a, int i, int j);

 protected:
  int num_rounds_;
  int rate_;
  int blocks_;
  ConditionWordPtr M[50][5][5];
  ConditionWordPtr A[50 + 1][5][5];
  ConditionWordPtr B[50][5][5];
  ConditionWordPtr K[24];
  ConditionWordPtr K0;
};

#endif  // KECCAK_H_

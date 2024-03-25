#include "ripemd128/ripemd128.h"

#include "bitslice_step.h"
#include "carry_step.h"
#include "functions.h"

#define BIT_CHECK(num, pos) ((num) & (1ull << (pos)))

const uint32_t Ripemd128::KA[4] = {0x00000000UL, 0x5a827999UL, 0x6ed9eba1UL,
                                   0x8f1bbcdcUL};
const uint32_t Ripemd128::KB[4] = {0x00000000UL, 0x6d703ef3UL, 0x5c4dd124UL,
                                   0x50a28be6UL};

const int Ripemd128::SA[64] = {
    11, 14, 15, 12, 5,  8,  7,  9,  11, 13, 14, 15, 6,  7,  9,  8,
    7,  6,  8,  13, 11, 9,  7,  15, 7,  12, 15, 9,  11, 7,  13, 12,
    11, 13, 6,  7,  14, 9,  13, 15, 14, 8,  13, 6,  5,  12, 7,  5,
    11, 12, 14, 15, 14, 15, 9,  8,  9,  14, 5,  6,  8,  6,  5,  12,
};

const int Ripemd128::SB[64] = {
    8,  9,  9,  11, 13, 15, 15, 5,  7,  7,  8,  11, 14, 14, 12, 6,
    9,  13, 15, 7,  12, 8,  9,  11, 7,  7,  12, 7,  6,  15, 13, 11,
    9,  7,  15, 11, 8,  6,  6,  14, 12, 13, 5,  14, 13, 13, 7,  5,
    15, 5,  8,  11, 14, 14, 6,  14, 6,  9,  12, 9,  12, 5,  15, 8};

const int Ripemd128::XA[64] = {
    //   0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
    0, 1,  2,  3,  4,  5,  6,  7, 8,  9, 10, 11, 12, 13, 14, 15,
    7, 4,  13, 1,  10, 6,  15, 3, 12, 0, 9,  5,  2,  14, 11, 8,
    3, 10, 14, 4,  9,  15, 8,  1, 2,  7, 0,  6,  13, 11, 5,  12,
    1, 9,  11, 10, 0,  8,  12, 4, 13, 3, 7,  15, 14, 5,  6,  2};

const int Ripemd128::XB[64] = {
    //   0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
    5,  14, 7, 0, 9, 2,  11, 4,  13, 6,  15, 8,  1,  10, 3,  12,
    6,  11, 3, 7, 0, 13, 5,  10, 14, 15, 8,  12, 4,  9,  1,  2,
    15, 5,  1, 3, 7, 14, 6,  9,  11, 8,  12, 2,  10, 0,  4,  13,
    8,  6,  4, 1, 3, 11, 15, 0,  5,  12, 2,  13, 9,  7,  10, 14};

const int Ripemd128::H = 4;

void Ripemd128::AddToOptions(cxxopts::Options& options) {}

Ripemd128::Ripemd128(cxxopts::Options& options)
    : Crypto(options["word-size"].as<int>()),
      num_rounds_(options["num-rounds"].as<int>()),
      A(tA + H),
      B(tB + H) {
  // message words
  for (int i = 0; i < 16; i++)
    W[i] = AddConditionWord("W", i, H + i * 2 + 1, 2);

  // chaining input
  for (int i = -H; i < 0; i++) A[i] = B[i] = AddConditionWord("A", i, H + i, 0);

  for (int i = 0; i < num_rounds_; i++) {
    // left stream
    F[i] = AddConditionWord("F", i, H + i * 2 + 0, 0, SUBWORD);
    if (i < 16)
      Add(new BitsliceStep<XOR<3>>(word_size_, A[i - 1], A[i - 2], A[i - 3],
                                   F[i]));
    else if (i < 32)
      Add(new BitsliceStep<IF>(word_size_, A[i - 1], A[i - 2], A[i - 3],
                               F[i]));  // IF(X,Y,Z)
    else if (i < 48)
      Add(new BitsliceStep<ONX>(word_size_, A[i - 1], A[i - 3], A[i - 2],
                                F[i]));
    else  // if (i < 64)
      Add(new BitsliceStep<IF>(word_size_, A[i - 3], A[i - 1], A[i - 2],
                               F[i]));  // IF(Z,X,Y)
    A[i] = AddConditionWord("A", i, H + i * 2 + 1, 0);
    Add(new CarryStep<ADD<4>>(word_size_, A[i - 4], F[i], W[XA[i]],
                              ConditionWordPtr(new ConditionWord(KA[i / 16])),
                              A[i]->Rotr(SA[i])));

    // right stream
    G[i] = AddConditionWord("G", i, H + i * 2 + 0, 1, SUBWORD);
    if (i < 16)
      Add(new BitsliceStep<IF>(word_size_, B[i - 3], B[i - 1], B[i - 2],
                               G[i]));  // IF(Z,X,Y)
    else if (i < 32)
      Add(new BitsliceStep<ONX>(word_size_, B[i - 1], B[i - 3], B[i - 2],
                                G[i]));
    else if (i < 48)
      Add(new BitsliceStep<IF>(word_size_, B[i - 1], B[i - 2], B[i - 3],
                               G[i]));  // IF(X,Y,Z)
    else                                // if (i < 64)
      Add(new BitsliceStep<XOR<3>>(word_size_, B[i - 1], B[i - 2], B[i - 3],
                                   G[i]));
    B[i] = AddConditionWord("B", i, H + i * 2 + 1, 1);
    Add(new CarryStep<ADD<4>>(
        word_size_, B[i - 4], G[i], W[XB[i]],
        ConditionWordPtr(new ConditionWord(KB[3 - (i / 16)])),
        B[i]->Rotr(SB[i])));
  }
}

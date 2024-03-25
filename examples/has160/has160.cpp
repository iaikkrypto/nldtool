#include "has160/has160.h"

#include "bitslice_step.h"
#include "carry_step.h"
#include "functions.h"

#define rotr32(x, n) (((x) >> (n)) | ((x) << (32 - (n))))

uint32_t K[4] = {0x00000000, 0x5a827999, 0x6ed9eba1, 0x8f1bbcdc};

uint32_t s2[4] = {10, 17, 25, 30};

static int s22[4][20] = {{0,  10, 10, 10, 10, 10, 10, 10, 10, 10,
                          10, 10, 10, 10, 10, 10, 10, 10, 10, 10},
                         {10, 17, 17, 17, 17, 17, 17, 17, 17, 17,
                          17, 17, 17, 17, 17, 17, 17, 17, 17, 17},
                         {17, 25, 25, 25, 25, 25, 25, 25, 25, 25,
                          25, 25, 25, 25, 25, 25, 25, 25, 25, 25},
                         {25, 30, 30, 30, 30, 30, 30, 30, 30, 30,
                          30, 30, 30, 30, 30, 30, 30, 30, 30, 30}};

static int s23[4][20] = {{0,  0,  10, 10, 10, 10, 10, 10, 10, 10,
                          10, 10, 10, 10, 10, 10, 10, 10, 10, 10},
                         {10, 10, 17, 17, 17, 17, 17, 17, 17, 17,
                          17, 17, 17, 17, 17, 17, 17, 17, 17, 17},
                         {17, 17, 25, 25, 25, 25, 25, 25, 25, 25,
                          25, 25, 25, 25, 25, 25, 25, 25, 25, 25},
                         {25, 25, 30, 30, 30, 30, 30, 30, 30, 30,
                          30, 30, 30, 30, 30, 30, 30, 30, 30, 30}};

static int s24[4][20] = {{0,  0,  0,  10, 10, 10, 10, 10, 10, 10,
                          10, 10, 10, 10, 10, 10, 10, 10, 10, 10},
                         {10, 10, 10, 17, 17, 17, 17, 17, 17, 17,
                          17, 17, 17, 17, 17, 17, 17, 17, 17, 17},
                         {17, 17, 17, 25, 25, 25, 25, 25, 25, 25,
                          25, 25, 25, 25, 25, 25, 25, 25, 25, 25},
                         {25, 25, 25, 30, 30, 30, 30, 30, 30, 30,
                          30, 30, 30, 30, 30, 30, 30, 30, 30, 30}};

static int s1[20] = {5, 11, 7, 15, 6, 13, 8, 14, 7, 12,
                     9, 11, 8, 15, 6, 12, 9, 14, 5, 13};

static int ordering[4][20] = {
    {18, 0, 1, 2, 3, 19, 4, 5, 6, 7, 16, 8, 9, 10, 11, 17, 12, 13, 14, 15},
    {18, 3, 6, 9, 12, 19, 15, 2, 5, 8, 16, 11, 14, 1, 4, 17, 7, 10, 13, 0},
    {18, 12, 5, 14, 7, 19, 0, 9, 2, 11, 16, 4, 13, 6, 15, 17, 8, 1, 10, 3},
    {18, 7, 2, 13, 8, 19, 3, 14, 9, 4, 16, 15, 10, 5, 0, 17, 11, 6, 1, 12}};

void Has160::AddToOptions(cxxopts::Options& options) {}

Has160::Has160(cxxopts::Options& options)
    : Crypto(options["word-size"].as<int>()),
      A(tA + 5),
      num_rounds_(options["num-rounds"].as<int>()) {
  for (int16_t i = -5; i < 0; i++) A[i] = AddConditionWord("A", i, 5 + i, 0);

  for (uint16_t i = 0; i < 16; i++)
    W[i] = AddConditionWord("W", i, 5 + i * 2 + 1, 1);
  for (uint16_t i = 16; i < 20; i++)
    W[i] = AddConditionWord("W", i, 5 + i * 2 + 1, 1, SUBWORD);
  Add(new BitsliceStep<XOR<4>>(word_size_, W[0], W[1], W[2], W[3], W[16]));
  Add(new BitsliceStep<XOR<4>>(word_size_, W[4], W[5], W[6], W[7], W[17]));
  Add(new BitsliceStep<XOR<4>>(word_size_, W[8], W[9], W[10], W[11], W[18]));
  Add(new BitsliceStep<XOR<4>>(word_size_, W[12], W[13], W[14], W[15], W[19]));

  for (uint16_t i = 0; i < 16; i++) W[i + 20] = W[i];
  for (uint16_t i = 36; i < 40; i++)
    W[i] = AddConditionWord("W", i, 5 + i * 2 + 1, 1, SUBWORD);
  Add(new BitsliceStep<XOR<4>>(word_size_, W[3], W[6], W[9], W[12], W[36]));
  Add(new BitsliceStep<XOR<4>>(word_size_, W[15], W[2], W[5], W[8], W[37]));
  Add(new BitsliceStep<XOR<4>>(word_size_, W[11], W[14], W[1], W[4], W[38]));
  Add(new BitsliceStep<XOR<4>>(word_size_, W[7], W[10], W[13], W[0], W[39]));

  for (uint16_t i = 0; i < 16; i++) W[i + 40] = W[i];
  for (uint16_t i = 56; i < 60; i++)
    W[i] = AddConditionWord("W", i, 5 + i * 2 + 1, 1, SUBWORD);
  Add(new BitsliceStep<XOR<4>>(word_size_, W[12], W[5], W[14], W[7], W[56]));
  Add(new BitsliceStep<XOR<4>>(word_size_, W[0], W[9], W[2], W[11], W[57]));
  Add(new BitsliceStep<XOR<4>>(word_size_, W[4], W[13], W[6], W[15], W[58]));
  Add(new BitsliceStep<XOR<4>>(word_size_, W[8], W[1], W[10], W[3], W[59]));

  for (uint16_t i = 0; i < 16; i++) W[i + 60] = W[i];
  for (uint16_t i = 76; i < 80; i++)
    W[i] = AddConditionWord("W", i, 5 + i * 2 + 1, 1, SUBWORD);
  Add(new BitsliceStep<XOR<4>>(word_size_, W[7], W[2], W[13], W[8], W[76]));
  Add(new BitsliceStep<XOR<4>>(word_size_, W[3], W[14], W[9], W[4], W[77]));
  Add(new BitsliceStep<XOR<4>>(word_size_, W[15], W[10], W[5], W[0], W[78]));
  Add(new BitsliceStep<XOR<4>>(word_size_, W[11], W[6], W[1], W[12], W[79]));

  for (uint16_t i = 0; i < num_rounds_; i++) {
    F_[i] = AddConditionWord("F", i, 5 + i * 2, 0, SUBWORD);
    A[i] = AddConditionWord("A", i, 5 + i * 2 + 1, 0);
    if (i <= 19) {
      Add(new BitsliceStep<IF>(word_size_, A[i - 2],
                               A[i - 3]->Rotl(s22[0][i % 20]),
                               A[i - 4]->Rotl(s23[0][i % 20]), F_[i]));
      Add(new CarryStep<ADD<4>>(word_size_, F_[i], W[ordering[0][i % 20] + 0],
                                A[i - 1]->Rotl(s1[i % 20]),
                                A[i - 5]->Rotl(s24[0][i % 20]), A[i]));
    } else if (i <= 39) {
      Add(new BitsliceStep<XOR<3>>(word_size_, A[i - 2],
                                   A[i - 3]->Rotl(s22[1][i % 20]),
                                   A[i - 4]->Rotl(s23[1][i % 20]), F_[i]));
      Add(new CarryStep<ADD<5>>(
          word_size_, F_[i], W[ordering[1][i % 20] + 20],
          A[i - 1]->Rotl(s1[i % 20]), A[i - 5]->Rotl(s24[1][i % 20]),
          ConditionWordPtr(new ConditionWord(K[1])), A[i]));
    } else if (i <= 59) {
      Add(new BitsliceStep<ONX>(word_size_, A[i - 2],
                                A[i - 3]->Rotl(s22[2][i % 20]),
                                A[i - 4]->Rotl(s23[2][i % 20]), F_[i]));
      Add(new CarryStep<ADD<5>>(
          word_size_, F_[i], W[ordering[2][i % 20] + 40],
          A[i - 1]->Rotl(s1[i % 20]), A[i - 5]->Rotl(s24[2][i % 20]),
          ConditionWordPtr(new ConditionWord(K[2])), A[i]));
    } else if (i <= 79) {
      Add(new BitsliceStep<XOR<3>>(word_size_, A[i - 2],
                                   A[i - 3]->Rotl(s22[3][i % 20]),
                                   A[i - 4]->Rotl(s23[3][i % 20]), F_[i]));
      Add(new CarryStep<ADD<5>>(
          word_size_, F_[i], W[ordering[3][i % 20] + 60],
          A[i - 1]->Rotl(s1[i % 20]), A[i - 5]->Rotl(s24[3][i % 20]),
          ConditionWordPtr(new ConditionWord(K[3])), A[i]));
    }
  }
}

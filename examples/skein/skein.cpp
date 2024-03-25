#include "skein/skein.h"

#include <cmath>
#include <iostream>
#include <string>

#include "bitslice_step.h"
#include "functions.h"
#include "horizontal_condition_word.h"

const uint32_t Skein::Rdj[8][2] = {{14, 16}, {52, 57}, {23, 40}, {5, 37},
                                   {25, 33}, {46, 12}, {58, 22}, {32, 32}};

void Skein::AddToOptions(cxxopts::Options &options) {}

Skein::Skein(cxxopts::Options &options)
    : Crypto(options["word-size"].as<int>()),
      num_rounds_(options["num-rounds"].as<int>()) {
  /* read from testvector */
  int i = 0;
  for (i = 0; i < WORD_SIZE; i++) {
    if (i < 3) {
      tweak[i] = AddConditionWord(std::string("t") + std::to_string(i), 0, -1,
                                  -1, HORIZONTALWORD);
      tweak2[i] = AddReferenceToConditionWord(
          tweak[i], std::string("t") + std::to_string(i), 0, i, 0, MAINWORD, 1);
    }
    if (i == 3) {
      key[4] = AddConditionWord(std::string("k") + std::to_string(4), 0, -1, -1,
                                HORIZONTALWORD);
      key2[4] = AddReferenceToConditionWord(
          key[4], std::string("k") + std::to_string(4), 0, i, 0, MAINWORD, 1);
    }
  }

  for (i = 0; i < WORD_SIZE; i++) {
    key[i] = AddConditionWord(std::string("k") + std::to_string(i), 0, -1, -1,
                              HORIZONTALWORD);
    key2[i] = AddReferenceToConditionWord(
        key[i], std::string("k") + std::to_string(i), 0, i + 4, 0, MAINWORD, 1);
  }

  int nr = 0;
  if (num_rounds_ < SKEIN_256_ROUNDS)
    nr = num_rounds_ + 1;
  else
    nr = SKEIN_256_ROUNDS;

  int j = 0;
  for (i = 0; i <= nr; i++) {
    for (j = 0; j < WORD_SIZE; j++) {
      e[i][j] =
          AddConditionWord(std::string(1, 0x41 + j), i, -1, -1, HORIZONTALWORD);
      v[i][j] =
          AddConditionWord(std::string(1, 0x56 + j), i, -1, -1, HORIZONTALWORD);

      e2[i][j] =
          AddReferenceToConditionWord(e[i][j], std::string(1, 0x41 + j), i,
                                      (4 * i) + 8 + j, 0, MAINWORD, 1);
      v2[i][j] =
          AddReferenceToConditionWord(v[i][j], std::string(1, 0x56 + j), i,
                                      (4 * i) + 8 + j, 1, MAINWORD, 1);
    }

    if (i % 4 == 0) {
      for (j = 0; j < WORD_SIZE; j++) {
        roundkeys[i / 4][j] = AddConditionWord(
            std::string("k") + std::to_string(j), i, -1, -1, HORIZONTALWORD);
        roundkeys2[i / 4][j] = AddReferenceToConditionWord(
            roundkeys[i / 4][j], std::string("k") + std::to_string(j), i,
            (4 * i) + 8 + j, 2, MAINWORD, 1);
      }
    }
  }

  /* key schedule */
  ConditionWordPtr C240 = ConditionWordPtr(
      new HorizontalConditionWord(0x1BD11BDAA9FC1A22, word_size_));
  Add(new BitsliceStep<XORB<2>>(word_size_ - 1, tweak[0], tweak[1], tweak[2]));
  Add(new BitsliceStep<XORB<5>>(word_size_ - 1, C240, key[0], key[1], key[2],
                                key[3], key[4]));

  if (num_rounds_ < SKEIN_256_ROUNDS)
    nr = ceil(((float)num_rounds_) / 4);
  else
    nr = 19;

  int s = 0;
  for (; s < nr; s++) {
    ConditionWordPtr c[4];
    for (int cw = 0; cw < 4; cw++)
      c[cw] = AddConditionWord("C", -1, -1, -1, CARRYWORD);

    Add(new BitsliceStep<ADDB<2>>(
        word_size_ - 1, key[(s + 0) % 5],
        ConditionWordPtr(new HorizontalConditionWord(0, word_size_)),
        c[0]->Shl(1), roundkeys[s][0], c[0], c[0]->Shr(1)));
    Add(new BitsliceStep<ADDB<2>>(word_size_ - 1, key[(s + 1) % 5],
                                  tweak[s % 3], c[1]->Shl(1), roundkeys[s][1],
                                  c[1], c[1]->Shr(1)));
    Add(new BitsliceStep<ADDB<2>>(word_size_ - 1, key[(s + 2) % 5],
                                  tweak[(s + 1) % 3], c[2]->Shl(1),
                                  roundkeys[s][2], c[2], c[2]->Shr(1)));
    Add(new BitsliceStep<ADDB<2>>(
        word_size_ - 1, key[(s + 3) % 5],
        ConditionWordPtr(new HorizontalConditionWord(s, word_size_)),
        c[3]->Shl(1), roundkeys[s][3], c[3], c[3]->Shr(1)));
  }

  int d = 0;
  for (; d < (num_rounds_ < SKEIN_256_ROUNDS ? num_rounds_ : SKEIN_256_ROUNDS);
       d++) {
    ConditionWordPtr c[6];
    for (int cw = 0; cw < 6; cw++)
      c[cw] = AddConditionWord("C", -1, -1, -1, CARRYWORD);

    if (d % 4 == 0) {
      // add roundkey
      for (i = 0; i < WORD_SIZE; i++) {
        Step *step = Add(new BitsliceStep<ADDB<2>>(
            word_size_ - 1, v[d][i], roundkeys[d / 4][i], c[i]->Shl(1), e[d][i],
            c[i], c[i]->Shr(1)));
        step->SetProbabilityMethod(BITSLICE);
        e[d][i]->SetStepToComputeProbability(step);
      }

    } else {
      for (i = 0; i < WORD_SIZE; i++) {
        Step *step = Add(new BitsliceStep<ADDB<2>>(
            word_size_ - 1, v[d][i],
            ConditionWordPtr(new ConditionWord(0, word_size_)), c[i]->Shl(1),
            e[d][i], c[i], c[i]->Shr(1)));
        step->SetProbabilityMethod(BITSLICE);
        e[d][i]->SetStepToComputeProbability(step);
      }
    }

    // mix & permutate
    Add(new BitsliceStep<ADDB<2>>(word_size_ - 1, e[d][0], e[d][1],
                                  c[4]->Shl(1), v[d + 1][0], c[4],
                                  c[4]->Shr(1)));
    Add(new BitsliceStep<XORB<2>>(word_size_ - 1, e[d][1]->Rotl(Rdj[d % 8][0]),
                                  v[d + 1][0], v[d + 1][3]));

    Add(new BitsliceStep<ADDB<2>>(word_size_ - 1, e[d][2], e[d][3],
                                  c[5]->Shl(1), v[d + 1][2], c[5],
                                  c[5]->Shr(1)));
    Add(new BitsliceStep<XORB<2>>(word_size_ - 1, e[d][3]->Rotl(Rdj[d % 8][1]),
                                  v[d + 1][2], v[d + 1][1]));
  }

  // final roundkey addition
  ConditionWordPtr c[4];
  for (int cw = 0; cw < 4; cw++)
    c[cw] = AddConditionWord("C", -1, -1, -1, CARRYWORD);

  if (num_rounds_ == SKEIN_256_ROUNDS) {
    for (i = 0; i < WORD_SIZE; i++)
      Add(new BitsliceStep<ADDB<2>>(
          word_size_ - 1, v[SKEIN_256_ROUNDS][i], roundkeys[18][i],
          c[i]->Shl(1), e[SKEIN_256_ROUNDS][i], c[i], c[i]->Shr(1)));
  }
}

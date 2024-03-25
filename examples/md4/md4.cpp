#include "md4/md4.h"

#include "bitslice_step.h"
#include "carry_step.h"
#include "functions.h"

const uint32_t Md4::K[3] = {0x00000000, 0x5a827999, 0x6ed9eba1};

const int Md4::S[12] = {3, 7, 11, 19, 3, 5, 9, 13, 3, 9, 11, 15};

const int Md4::P[48] = {
    0, 1, 2, 3,  4, 5,  6, 7,  8, 9, 10, 11, 12, 13, 14, 15,
    0, 4, 8, 12, 1, 5,  9, 13, 2, 6, 10, 14, 3,  7,  11, 15,
    0, 8, 4, 12, 2, 10, 6, 14, 1, 9, 5,  13, 3,  11, 7,  15,
};

void Md4::AddToOptions(cxxopts::Options& options) {}

Md4::Md4(cxxopts::Options& options)
    : Crypto(options["word-size"].as<int>()),
      num_rounds_(options["num-rounds"].as<int>()) {
  Step* step = 0;

  for (int i = -4; i < 0; i++) A[i] = AddConditionWord("A", i, 4 + i, 0);
  for (int i = 0; i < std::min(16, num_rounds_); i++)
    W[i] = AddConditionWord("W", i, 4 + i * 2 + 1, 1);

  for (int i = 0; i < num_rounds_; i++) {
    F[i] = AddConditionWord("F", i, 4 + i * 2 + 0, 1, SUBWORD);
    A[i] = AddConditionWord("A", i, 4 + i * 2 + 1, 0);

    if (i < 16)
      step = Add(
          new BitsliceStep<IF>(word_size_, A[i - 1], A[i - 2], A[i - 3], F[i]));
    else if (i < 32)
      step = Add(new BitsliceStep<MAJ>(word_size_, A[i - 1], A[i - 2], A[i - 3],
                                       F[i]));
    else
      step = Add(new BitsliceStep<XOR<3>>(word_size_, A[i - 1], A[i - 2],
                                          A[i - 3], F[i]));
    F[i]->SetStepToComputeProbability(step);

    int m = P[i];
    ConditionWordPtr k(new ConditionWord(K[i / 16]));
    step = Add(new CarryStep<ADD<4>>(word_size_, A[i - 4], F[i], W[m], k,
                                     A[i]->Rotr(S[(i / 16) * 4 + i % 4])));
    step->SetProbabilityMethod(CYCLICGRAPH);
    A[i]->SetStepToComputeProbability(step);
  }
}

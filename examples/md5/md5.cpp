#include "md5/md5.h"

#include "bitslice_step.h"
#include "functions.h"

const uint32_t Md5::K[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a,
    0xa8304613, 0xfd469501, 0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821, 0xf61e2562, 0xc040b340,
    0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8,
    0x676f02d9, 0x8d2a4c8a, 0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70, 0x289b7ec6, 0xeaa127fa,
    0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92,
    0xffeff47d, 0x85845dd1, 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

const int Md5::S[64] = {7,  12, 17, 22, 7,  12, 17, 22, 7,  12, 17, 22, 7,
                        12, 17, 22, 5,  9,  14, 20, 5,  9,  14, 20, 5,  9,
                        14, 20, 5,  9,  14, 20, 4,  11, 16, 23, 4,  11, 16,
                        23, 4,  11, 16, 23, 4,  11, 16, 23, 6,  10, 15, 21,
                        6,  10, 15, 21, 6,  10, 15, 21, 6,  10, 15, 21};

const int Md5::H = 4;
const int Md5::L = 7;

class Md5::ADD4ADD2 : public F {
 public:
  static constexpr char kName[] = "ADD4ADD2";
  static const int kNumInputs = 6;
  static const int kNumOutputs = 3;
  static const int kPrevState = 5;
  static const int kNextState = 8;
  static const int kStateSize = 64;
  static constexpr int Symmetry(int i) { return (i < 4) ? 0 : i; }
  static constexpr int Bitsize(int i) { return (i == 5 || i == 8) ? 3 : 1; }
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    const int ci = kNumInputs - 1;
    const int sb = 0;
    const int sa = 1;
    const int co = 2;
    y[sb] = x[0] + x[1] + x[2] + x[3] + (x[ci] % 4);
    y[co] = y[sb] >> 1;
    y[sb] &= 1;
    y[sa] = y[sb] + x[4] + (x[ci] / 4);
    y[co] |= (y[sa] >> 1) * 4;
    y[sa] &= 1;
  }
};

constexpr char Md5::ADD4ADD2::kName[];

void Md5::AddToOptions(cxxopts::Options& options) {}

Md5::Md5(cxxopts::Options& options)
    : Crypto(options["word-size"].as<int>()),
      num_rounds_(options["num-rounds"].as<int>()),
      A(tA + H) {
  for (int i = -H; i < 0; i++) A[i] = AddConditionWord("A", i, H + i, 0);
  for (int i = 0; i < std::min(16, num_rounds_); i++)
    W[i] = AddConditionWord("W", i, 4 + i * L + 1, 1);

  Step* step;
  for (int i = 0; i < num_rounds_; i++) {
    int m;
    if (i < 16)
      m = i;
    else if (i < 32)
      m = (5 * i + 1) % 16;
    else if (i < 48)
      m = (3 * i + 5) % 16;
    else /* if (i < 64) */
      m = (7 * i) % 16;

    B[i] = AddConditionWord("B", i, 4 + i * L + 0, 0, SUBWORD);
    F[i] = AddConditionWord("F", i, 4 + i * L + 0, 1, SUBWORD);
    A[i] = AddConditionWord("A", i, 4 + i * L + 1, 0);
    ConditionWordPtr k(new ConditionWord(K[i]));
    ConditionWordPtr c3o =
        AddConditionWord("Co", i, 4 + i * L + 3, 0, CARRYWORD, 3);

    ConditionWordPtr c3i = c3o->Rotl(1)->SetZero(0, 1, 2)->SetZero(S[i], 2, 0);

    if (i < 16)
      step = Add(new BitsliceStep<IF>(word_size_, A[i - 1], A[i - 2], A[i - 3],
                                      F[i]));  // IF(X,Y,Z)
    else if (i < 32)
      step = Add(new BitsliceStep<IF>(word_size_, A[i - 3], A[i - 1], A[i - 2],
                                      F[i]));  // IF(Z,X,Y)
    else if (i < 48)
      step = Add(new BitsliceStep<XOR<3>>(word_size_, A[i - 1], A[i - 2],
                                          A[i - 3], F[i]));
    else /* if (i < 64) */
      step = Add(new BitsliceStep<ONX>(word_size_, A[i - 1], A[i - 2], A[i - 3],
                                       F[i]));
    step->SetProbabilityMethod(BITSLICE);
    F[i]->SetStepToComputeProbability(step);

    ConditionWordPtr cb = AddReferenceToConditionWord(
        c3o->Rotr(S[i]), "CB", i, 4 + i * L + 2, 1, CARRYWORD, 2, 0);
    ConditionWordPtr ca = AddReferenceToConditionWord(
        c3o, "CA", i, 4 + i * L + 2, 0, CARRYWORD, 1, 2);
    Add(new BitsliceStep<ADD<4>>(word_size_, A[i - 4], F[i], W[m], k,
                                 cb->Shl(1), B[i], cb));
    Add(new BitsliceStep<ADD<2>>(word_size_, B[i]->Rotl(S[i]), A[i - 1],
                                 ca->Shl(1), A[i], ca));

    step = Add(new BitsliceStep<ADD4ADD2>(
        word_size_, A[i - 4]->Rotl(S[i]), F[i]->Rotl(S[i]), W[m]->Rotl(S[i]),
        k->Rotl(S[i]), A[i - 1], c3i, B[i]->Rotl(S[i]), A[i], c3o));

    step->SetProbabilityMethod(CYCLICGRAPH);
    A[i]->SetStepToComputeProbability(step);
  }
}

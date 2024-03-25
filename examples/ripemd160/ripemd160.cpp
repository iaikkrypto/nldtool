#include "ripemd160/ripemd160.h"

#include "bitslice_step.h"
#include "carry_step.h"
#include "functions.h"

const unsigned int Ripemd160::KA[5] = {0x00000000, 0x5a827999, 0x6ed9eba1,
                                       0x8f1bbcdc, 0xa953fd4e};
const unsigned int Ripemd160::KB[5] = {0x00000000, 0x7a6d76e9, 0x6d703ef3,
                                       0x5c4dd124, 0x50a28be6};

const int Ripemd160::SA[80] = {
    11, 14, 15, 12, 5,  8,  7,  9,  11, 13, 14, 15, 6,  7,  9,  8,
    7,  6,  8,  13, 11, 9,  7,  15, 7,  12, 15, 9,  11, 7,  13, 12,
    11, 13, 6,  7,  14, 9,  13, 15, 14, 8,  13, 6,  5,  12, 7,  5,
    11, 12, 14, 15, 14, 15, 9,  8,  9,  14, 5,  6,  8,  6,  5,  12,
    9,  15, 5,  11, 6,  8,  13, 12, 5,  12, 13, 14, 11, 8,  5,  6};

const int Ripemd160::SB[80] = {
    8,  9,  9,  11, 13, 15, 15, 5,  7,  7,  8,  11, 14, 14, 12, 6,
    9,  13, 15, 7,  12, 8,  9,  11, 7,  7,  12, 7,  6,  15, 13, 11,
    9,  7,  15, 11, 8,  6,  6,  14, 12, 13, 5,  14, 13, 13, 7,  5,
    15, 5,  8,  11, 14, 14, 6,  14, 6,  9,  12, 9,  12, 5,  15, 8,
    8,  5,  12, 9,  12, 5,  14, 6,  8,  13, 6,  5,  15, 13, 11, 11};

const int Ripemd160::XA[80] = {
    0,  1, 2,  3, 4,  5,  6, 7,  8, 9,  10, 11, 12, 13, 14, 15, 7,  4,  13, 1,
    10, 6, 15, 3, 12, 0,  9, 5,  2, 14, 11, 8,  3,  10, 14, 4,  9,  15, 8,  1,
    2,  7, 0,  6, 13, 11, 5, 12, 1, 9,  11, 10, 0,  8,  12, 4,  13, 3,  7,  15,
    14, 5, 6,  2, 4,  0,  5, 9,  7, 12, 2,  10, 14, 1,  3,  8,  11, 6,  15, 13};

const int Ripemd160::XB[80] = {
    5,  14, 7,  0,  9,  2,  11, 4,  13, 6, 15, 8, 1,  10, 3,  12, 6, 11, 3, 7,
    0,  13, 5,  10, 14, 15, 8,  12, 4,  9, 1,  2, 15, 5,  1,  3,  7, 14, 6, 9,
    11, 8,  12, 2,  10, 0,  4,  13, 8,  6, 4,  1, 3,  11, 15, 0,  5, 12, 2, 13,
    9,  7,  10, 14, 12, 15, 10, 4,  1,  5, 8,  7, 6,  2,  13, 14, 0, 3,  9, 11};

const int Ripemd160::H = 5;
const int Ripemd160::L = 6;

class Ripemd160::ADD4ADD2 : public F {
 public:
  static constexpr char kName[] = "ADD4ADD2";
  static const int kNumInputs = 6;
  static const int kNumOutputs = 3;
  static constexpr int Symmetry(int i) { return (i < 4) ? 0 : i; }
  static constexpr int Bitsize(int i) { return (i == 5 || i == 8) ? 3 : 1; }
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    const int ci = kNumInputs - 1;
    const int sb = 0;
    const int sa = 1;
    const int co = 2;
    y[sb] = x[0] + x[1] + x[2] + x[3] + ((x[ci] >> 1) & 3);
    y[co] = y[sb] & 6;
    y[sb] &= 1;
    y[sa] = y[sb] + x[4] + (x[ci] & 1);
    y[co] |= y[sa] >> 1;
    y[sa] &= 1;
  }
};

constexpr char Ripemd160::ADD4ADD2::kName[];

class Ripemd160::XOR3ADD4ADD2 : public F {
 public:
  static constexpr char kName[] = "XOR3ADD4ADD2";
  static const int kNumInputs = 8;
  static const int kNumOutputs = 4;
  static const int kPrevState = 7;
  static const int kNextState = 11;
  static const int kStateSize = 64;
  static constexpr int Symmetry(int i) { return (i < 3) ? 0 : (i < 6) ? 1 : i; }
  static constexpr int Bitsize(int i) { return (i == 7 || i == 11) ? 3 : 1; }
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    const int ci = kNumInputs - 1;
    const int f = 0;
    const int sb = 1;
    const int sa = 2;
    const int co = 3;
    y[f] = x[0] ^ x[1] ^ x[2];
    y[sb] = y[f] + x[3] + x[4] + x[5] + ((x[ci] >> 1) & 3);
    y[co] = y[sb] & 6;
    y[sb] &= 1;
    y[sa] = y[sb] + x[6] + (x[ci] & 1);
    y[co] |= y[sa] >> 1;
    y[sa] &= 1;
  }
};

constexpr char Ripemd160::XOR3ADD4ADD2::kName[];

class Ripemd160::IFADD4ADD2 : public F {
 public:
  static constexpr char kName[] = "IFADD4ADD2";
  static const int kNumInputs = 8;
  static const int kNumOutputs = 4;
  static const int kPrevState = 7;
  static const int kNextState = 11;
  static const int kStateSize = 64;
  static constexpr int Symmetry(int i) { return (3 <= i && i < 6) ? 3 : i; }
  static constexpr int Bitsize(int i) { return (i == 7 || i == 11) ? 3 : 1; }
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    const int ci = kNumInputs - 1;
    const int f = 0;
    const int sb = 1;
    const int sa = 2;
    const int co = 3;
    y[f] = (x[0] & x[1]) | ((!x[0]) & x[2]);
    y[sb] = y[f] + x[3] + x[4] + x[5] + ((x[ci] >> 1) & 3);
    y[co] = y[sb] & 6;
    y[sb] &= 1;
    y[sa] = y[sb] + x[6] + (x[ci] & 1);
    y[co] |= y[sa] >> 1;
    y[sa] &= 1;
  }
};

constexpr char Ripemd160::IFADD4ADD2::kName[];

class Ripemd160::ONXADD4ADD2 : public F {
 public:
  static constexpr char kName[] = "IFADD4ADD2";
  static const int kNumInputs = 8;
  static const int kNumOutputs = 4;
  static const int kPrevState = 7;
  static const int kNextState = 11;
  static const int kStateSize = 64;
  static constexpr int Symmetry(int i) { return (3 <= i && i < 6) ? 3 : i; }
  static constexpr int Bitsize(int i) { return (i == 7 || i == 11) ? 3 : 1; }
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    const int ci = kNumInputs - 1;
    const int f = 0;
    const int sb = 1;
    const int sa = 2;
    const int co = 3;
    y[f] = x[1] ^ (x[0] | (!x[2]));
    y[sb] = y[f] + x[3] + x[4] + x[5] + ((x[ci] >> 1) & 3);
    y[co] = y[sb] & 6;
    y[sb] &= 1;
    y[sa] = y[sb] + x[6] + (x[ci] & 1);
    y[co] |= y[sa] >> 1;
    y[sa] &= 1;
  }
};

constexpr char Ripemd160::ONXADD4ADD2::kName[];

void Ripemd160::AddToOptions(cxxopts::Options& options) {
  options.add_options("ripemd160 specific")        //
      ("ripemd160-bs",                             //
       "bitslice options",                         //
       cxxopts::value<int>()->default_value("7"),  //
       "N");
}

Ripemd160::Ripemd160(cxxopts::Options& options)
    : Crypto(options["word-size"].as<int>()),
      num_rounds_(options["num-rounds"].as<int>()),
      start_round_(options["start-round"].as<int>()),
      A(tA + H),
      B(tB + H) {
  int bitslice_flags = options["ripemd160-bs"].as<int>();
  // setup chaining value words and message words
  for (int i = -H; i < 0; i++) A[i] = B[i] = AddConditionWord("A", i, H + i, 0);
  for (int i = 0; i < 16; i++)
    W[i] = AddConditionWord("W", i, H + i * L + 3, 2);

  // setup all characteristic words
  for (int i = 0; i < num_rounds_; i++) {
    // words for left stream
    F[i] = AddConditionWord("F", i, H + i * L + 0, 0, SUBWORD);
    D[i] = AddConditionWord("D", i, H + i * L + 1, 0, SUBWORD);
    A[i] = AddConditionWord("A", i, H + i * L + 3, 0);
    CAo[i] = AddConditionWord("CA3", i, H + i * L + 5, 0, CARRYWORD, 3);
    CAi[i] = CAo[i]->Rotl(1)->SetZero(0, 1, 0)->SetZero(SA[i], 2, 1);
    CD[i] = AddReferenceToConditionWord(CAo[i]->Rotr(SA[i]), "CD", i,
                                        H + i * L + 2, 0, CARRYWORD, 2, 1);
    CA[i] = AddReferenceToConditionWord(CAo[i], "CA", i, H + i * L + 4, 0,
                                        CARRYWORD, 1, 0);
    // words for right stream
    G[i] = AddConditionWord("G", i, H + i * L + 0, 1, SUBWORD);
    E[i] = AddConditionWord("E", i, H + i * L + 1, 1, SUBWORD);
    B[i] = AddConditionWord("B", i, H + i * L + 3, 1);
    CBo[i] = AddConditionWord("CB3", i, H + i * L + 5, 1, CARRYWORD, 3);
    CBi[i] = CBo[i]->Rotl(1)->SetZero(0, 1, 0)->SetZero(SB[i], 2, 1);
    CE[i] = AddReferenceToConditionWord(CBo[i]->Rotr(SB[i]), "CE", i,
                                        H + i * L + 2, 1, CARRYWORD, 2, 1);
    CB[i] = AddReferenceToConditionWord(CBo[i], "CB", i, H + i * L + 4, 1,
                                        CARRYWORD, 1, 0);
  }

  // correct chaining value if we start at a later step
  for (int i = 0; i < H; i++)
    A[start_round_ - H + i] = B[start_round_ - H + i] = tA[i];

  int r, s, t;
  Step* step;
  for (int i = start_round_; i < num_rounds_; i++) {
    // left stream
    K = ConditionWordPtr(new ConditionWord(KA[i / 16]));
    r = (i < start_round_ + 1) ? 0 : 10;
    if (i < 16)
      Add(new BitsliceStep<XOR<3>>(word_size_, A[i - 1], A[i - 2],
                                   A[i - 3]->Rotl(r),
                                   F[i]));  // XOR(X,Y,Z)
    else if (i < 32)
      Add(new BitsliceStep<IF>(word_size_, A[i - 1], A[i - 2],
                               A[i - 3]->Rotl(r),
                               F[i]));  //  IF(X,Y,Z)
    else if (i < 48)
      Add(new BitsliceStep<ONX>(word_size_, A[i - 1], A[i - 3]->Rotl(r),
                                A[i - 2],
                                F[i]));  // ONX(X,Z,Y)
    else if (i < 64)
      Add(new BitsliceStep<IF>(word_size_, A[i - 3]->Rotl(r), A[i - 1],
                               A[i - 2],
                               F[i]));  //  IF(Z,X,Y)
    else
      Add(new BitsliceStep<ONX>(word_size_, A[i - 2], A[i - 1],
                                A[i - 3]->Rotl(r),
                                F[i]));  // ONX(Y,X,Z)

    if (bitslice_flags & 1) {
      r = (i < start_round_ + 3) ? 0 : 10;
      Add(new BitsliceStep<ADD<4>>(word_size_, A[i - 5]->Rotl(r), F[i],
                                   W[XA[i]], K, CD[i]->Shl(1), D[i], CD[i]));
      r = (i < start_round_ + 2) ? 0 : 10;
      Add(new BitsliceStep<ADD<2>>(word_size_, D[i]->Rotl(SA[i]),
                                   A[i - 4]->Rotl(r), CA[i]->Shl(1), A[i],
                                   CA[i]));
    }

    if (bitslice_flags & 2) {
      r = (i < start_round_ + 2) ? 0 : 10;
      s = (i <= start_round_ + 2) ? 0 : 10;
      Add(new BitsliceStep<ADD4ADD2>(word_size_, A[i - 5]->Rotl(s + SA[i]),
                                     F[i]->Rotl(SA[i]), W[XA[i]]->Rotl(SA[i]),
                                     K->Rotl(SA[i]), A[i - 4]->Rotl(r), CAi[i],
                                     D[i]->Rotl(SA[i]), A[i], CAo[i]));
    }

    if (bitslice_flags & 4) {
      r = (i < start_round_ + 2) ? 0 : 10;
      s = (i <= start_round_ + 2) ? 0 : 10;
      t = (i < start_round_ + 1) ? 0 : 10;
      if (i < 16)
        step = Add(new BitsliceStep<XOR3ADD4ADD2>(
            word_size_, A[i - 1]->Rotl(SA[i]), A[i - 2]->Rotl(SA[i]),
            A[i - 3]->Rotl(t + SA[i]), A[i - 5]->Rotl(s + SA[i]),
            W[XA[i]]->Rotl(SA[i]), K->Rotl(SA[i]), A[i - 4]->Rotl(r), CAi[i],
            F[i]->Rotl(SA[i]), D[i]->Rotl(SA[i]), A[i], CAo[i]));
      else if (i < 32)
        step = Add(new BitsliceStep<IFADD4ADD2>(
            word_size_, A[i - 1]->Rotl(SA[i]), A[i - 2]->Rotl(SA[i]),
            A[i - 3]->Rotl(t + SA[i]), A[i - 5]->Rotl(s + SA[i]),
            W[XA[i]]->Rotl(SA[i]), K->Rotl(SA[i]), A[i - 4]->Rotl(r), CAi[i],
            F[i]->Rotl(SA[i]), D[i]->Rotl(SA[i]), A[i], CAo[i]));
      else if (i < 48)
        step = Add(new BitsliceStep<ONXADD4ADD2>(
            word_size_, A[i - 1]->Rotl(SA[i]), A[i - 3]->Rotl(t + SA[i]),
            A[i - 2]->Rotl(SA[i]), A[i - 5]->Rotl(s + SA[i]),
            W[XA[i]]->Rotl(SA[i]), K->Rotl(SA[i]), A[i - 4]->Rotl(r), CAi[i],
            F[i]->Rotl(SA[i]), D[i]->Rotl(SA[i]), A[i], CAo[i]));
      else if (i < 64)
        step = Add(new BitsliceStep<IFADD4ADD2>(
            word_size_, A[i - 3]->Rotl(t + SA[i]), A[i - 1]->Rotl(SA[i]),
            A[i - 2]->Rotl(SA[i]), A[i - 5]->Rotl(s + SA[i]),
            W[XA[i]]->Rotl(SA[i]), K->Rotl(SA[i]), A[i - 4]->Rotl(r), CAi[i],
            F[i]->Rotl(SA[i]), D[i]->Rotl(SA[i]), A[i], CAo[i]));
      else
        step = Add(new BitsliceStep<ONXADD4ADD2>(
            word_size_, A[i - 2]->Rotl(SA[i]), A[i - 1]->Rotl(SA[i]),
            A[i - 3]->Rotl(t + SA[i]), A[i - 5]->Rotl(s + SA[i]),
            W[XA[i]]->Rotl(SA[i]), K->Rotl(SA[i]), A[i - 4]->Rotl(r), CAi[i],
            F[i]->Rotl(SA[i]), D[i]->Rotl(SA[i]), A[i], CAo[i]));
      step->SetProbabilityMethod(CYCLICGRAPH);
      A[i]->SetStepToComputeProbability(step);
    }

    // right stream
    K = ConditionWordPtr(new ConditionWord(KB[4 - (i / 16)]));
    r = (i < start_round_ + 1) ? 0 : 10;
    if (i < 16)
      Add(new BitsliceStep<ONX>(word_size_, B[i - 2], B[i - 1],
                                B[i - 3]->Rotl(r),
                                G[i]));  // ONX(Y,X,Z)
    else if (i < 32)
      Add(new BitsliceStep<IF>(word_size_, B[i - 3]->Rotl(r), B[i - 1],
                               B[i - 2],
                               G[i]));  //  IF(Z,X,Y)
    else if (i < 48)
      Add(new BitsliceStep<ONX>(word_size_, B[i - 1], B[i - 3]->Rotl(r),
                                B[i - 2],
                                G[i]));  // ONX(X,Z,Y)
    else if (i < 64)
      Add(new BitsliceStep<IF>(word_size_, B[i - 1], B[i - 2],
                               B[i - 3]->Rotl(r),
                               G[i]));  //  IF(X,Y,Z)
    else
      Add(new BitsliceStep<XOR<3>>(word_size_, B[i - 1], B[i - 2],
                                   B[i - 3]->Rotl(r),
                                   G[i]));  // XOR(X,Y,Z)

    if (bitslice_flags & 1) {
      r = (i < start_round_ + 3) ? 0 : 10;
      Add(new BitsliceStep<ADD<4>>(word_size_, B[i - 5]->Rotl(r), G[i],
                                   W[XB[i]], K, CE[i]->Shl(1), E[i], CE[i]));
      r = (i < start_round_ + 2) ? 0 : 10;
      Add(new BitsliceStep<ADD<2>>(word_size_, E[i]->Rotl(SB[i]),
                                   B[i - 4]->Rotl(r), CB[i]->Shl(1), B[i],
                                   CB[i]));
    }

    if (bitslice_flags & 2) {
      r = (i < start_round_ + 2) ? 0 : 10;
      s = (i <= start_round_ + 2) ? 0 : 10;
      Add(new BitsliceStep<ADD4ADD2>(word_size_, B[i - 5]->Rotl(s + SB[i]),
                                     G[i]->Rotl(SB[i]), W[XB[i]]->Rotl(SB[i]),
                                     K->Rotl(SB[i]), B[i - 4]->Rotl(r), CBi[i],
                                     E[i]->Rotl(SB[i]), B[i], CBo[i]));
    }

    if (bitslice_flags & 4) {
      r = (i < start_round_ + 2) ? 0 : 10;
      s = (i <= start_round_ + 2) ? 0 : 10;
      t = (i < start_round_ + 1) ? 0 : 10;
      if (i < 16)
        step = Add(new BitsliceStep<ONXADD4ADD2>(
            word_size_, B[i - 2]->Rotl(SB[i]), B[i - 1]->Rotl(SB[i]),
            B[i - 3]->Rotl(t + SB[i]), B[i - 5]->Rotl(s + SB[i]),
            W[XB[i]]->Rotl(SB[i]), K->Rotl(SB[i]), B[i - 4]->Rotl(r), CBi[i],
            G[i]->Rotl(SB[i]), E[i]->Rotl(SB[i]), B[i], CBo[i]));
      else if (i < 32)
        step = Add(new BitsliceStep<IFADD4ADD2>(
            word_size_, B[i - 3]->Rotl(t + SB[i]), B[i - 1]->Rotl(SB[i]),
            B[i - 2]->Rotl(SB[i]), B[i - 5]->Rotl(s + SB[i]),
            W[XB[i]]->Rotl(SB[i]), K->Rotl(SB[i]), B[i - 4]->Rotl(r), CBi[i],
            G[i]->Rotl(SB[i]), E[i]->Rotl(SB[i]), B[i], CBo[i]));
      else if (i < 48)
        step = Add(new BitsliceStep<ONXADD4ADD2>(
            word_size_, B[i - 1]->Rotl(SB[i]), B[i - 3]->Rotl(t + SB[i]),
            B[i - 2]->Rotl(SB[i]), B[i - 5]->Rotl(s + SB[i]),
            W[XB[i]]->Rotl(SB[i]), K->Rotl(SB[i]), B[i - 4]->Rotl(r), CBi[i],
            G[i]->Rotl(SB[i]), E[i]->Rotl(SB[i]), B[i], CBo[i]));
      else if (i < 64)
        step = Add(new BitsliceStep<IFADD4ADD2>(
            word_size_, B[i - 1]->Rotl(SB[i]), B[i - 2]->Rotl(SB[i]),
            B[i - 3]->Rotl(t + SB[i]), B[i - 5]->Rotl(s + SB[i]),
            W[XB[i]]->Rotl(SB[i]), K->Rotl(SB[i]), B[i - 4]->Rotl(r), CBi[i],
            G[i]->Rotl(SB[i]), E[i]->Rotl(SB[i]), B[i], CBo[i]));
      else
        step = Add(new BitsliceStep<XOR3ADD4ADD2>(
            word_size_, B[i - 1]->Rotl(SB[i]), B[i - 2]->Rotl(SB[i]),
            B[i - 3]->Rotl(t + SB[i]), B[i - 5]->Rotl(s + SB[i]),
            W[XB[i]]->Rotl(SB[i]), K->Rotl(SB[i]), B[i - 4]->Rotl(r), CBi[i],
            G[i]->Rotl(SB[i]), E[i]->Rotl(SB[i]), B[i], CBo[i]));
      step->SetProbabilityMethod(CYCLICGRAPH);
      B[i]->SetStepToComputeProbability(step);
    }
  }

  // feed-forward
  for (int i = num_rounds_; i < num_rounds_ + H; i++)
    A[i] = AddConditionWord("FA", i - num_rounds_,
                            H + num_rounds_ * L + i - num_rounds_, 0);
  //  h[0] = h[1] + C + D'
  step = Add(new CarryStep<ADD<3>>(
      word_size_, A[-H + 4], A[num_rounds_ - H + 3],
      B[num_rounds_ - H + 2]->Rotl(10), A[num_rounds_ + 0]));
  step->SetProbabilityMethod(CYCLICGRAPH);
  A[num_rounds_ + 0]->SetStepToComputeProbability(step);
  //  h[1] = h[2] + D + E'
  step = Add(new CarryStep<ADD<3>>(
      word_size_, A[-H + 3], A[num_rounds_ - H + 2]->Rotl(10),
      B[num_rounds_ - H + 1]->Rotl(10), A[num_rounds_ + 4]));
  step->SetProbabilityMethod(CYCLICGRAPH);
  A[num_rounds_ + 4]->SetStepToComputeProbability(step);
  //  h[2] = h[3] + E + A'
  step = Add(new CarryStep<ADD<3>>(
      word_size_, A[-H + 2], A[num_rounds_ - H + 1]->Rotl(10),
      B[num_rounds_ - H + 0]->Rotl(10), A[num_rounds_ + 3]));
  step->SetProbabilityMethod(CYCLICGRAPH);
  A[num_rounds_ + 3]->SetStepToComputeProbability(step);
  //  h[3] = h[4] + A + B'
  step = Add(new CarryStep<ADD<3>>(word_size_, A[-H + 1],
                                   A[num_rounds_ - H + 0]->Rotl(10),
                                   B[num_rounds_ - H + 4], A[num_rounds_ + 2]));
  step->SetProbabilityMethod(CYCLICGRAPH);
  A[num_rounds_ + 2]->SetStepToComputeProbability(step);
  //  h[4] = h[0] + B + C'
  step =
      Add(new CarryStep<ADD<3>>(word_size_, A[-H + 0], A[num_rounds_ - H + 4],
                                B[num_rounds_ - H + 3], A[num_rounds_ + 1]));
  step->SetProbabilityMethod(CYCLICGRAPH);
  A[num_rounds_ + 1]->SetStepToComputeProbability(step);
}

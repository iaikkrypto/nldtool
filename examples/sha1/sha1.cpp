#include "sha1/sha1.h"

#include "bitslice_step.h"
#include "functions.h"
#include "linear_step.h"

//#define CORRECTIV
//#define FEEDFORWARD

class Sha1::IFADD5 : public F {
 public:
  static constexpr char kName[] = "IFADD5";
  static const int kNumInputs = 8;
  static const int kNumOutputs = 3;
  static const int kPrevState = 7;
  static const int kNextState = 10;
  static const int kStateSize = 25;
  static constexpr int Symmetry(int i) { return (3 <= i && i < 7) ? 3 : i; }
  static constexpr int Bitsize(int i) { return (i == 7 || i == 10) ? 3 : 1; }
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    const int ci = kNumInputs - 1;
    const int f = 0;
    const int s = 1;
    const int co = 2;
    y[f] = (x[0] & x[1]) | ((!x[0]) & x[2]);
    y[s] = y[f] + x[3] + x[4] + x[5] + x[6] + x[ci];
    y[co] = y[s] >> 1;
    y[s] &= 1;
  }
};

constexpr char Sha1::IFADD5::kName[];

class Sha1::MAJADD5 : public F {
 public:
  static constexpr char kName[] = "MAJADD5";
  static const int kNumInputs = 8;
  static const int kNumOutputs = 3;
  static const int kPrevState = 7;
  static const int kNextState = 10;
  static const int kStateSize = 25;
  static constexpr int Symmetry(int i) { return (3 <= i && i < 7) ? 3 : i; }
  static constexpr int Bitsize(int i) { return (i == 7 || i == 10) ? 3 : 1; }
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    const int ci = kNumInputs - 1;
    const int f = 0;
    const int s = 1;
    const int co = 2;
    y[f] = (x[0] & x[1]) ^ (x[1] & x[2]) ^ (x[2] & x[0]);
    y[s] = y[f] + x[3] + x[4] + x[5] + x[6] + x[ci];
    y[co] = y[s] >> 1;
    y[s] &= 1;
  }
};

constexpr char Sha1::MAJADD5::kName[];

class Sha1::XOR3ADD5 : public F {
 public:
  static constexpr char kName[] = "XOR3ADD5";
  static const int kNumInputs = 8;
  static const int kNumOutputs = 3;
  static const int kPrevState = 7;
  static const int kNextState = 10;
  static const int kStateSize = 25;
  static constexpr int Symmetry(int i) { return (i < 3) ? 0 : (i < 7) ? 3 : i; }
  static constexpr int Bitsize(int i) { return (i == 7 || i == 10) ? 3 : 1; }
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    const int ci = kNumInputs - 1;
    const int f = 0;
    const int s = 1;
    const int co = 2;
    y[f] = x[0] ^ x[1] ^ x[2];
    y[s] = y[f] + x[3] + x[4] + x[5] + x[6] + x[ci];
    y[co] = y[s] >> 1;
    y[s] &= 1;
  }
};

constexpr char Sha1::XOR3ADD5::kName[];

class Sha1::MEXP : public F {
 public:
  static constexpr char kName[] = "MEXP";
  static const int kNumInputs = 16;
  static const int kNumOutputs = 64;
  template <class T>
  static void f(int word_size, const T x[kNumInputs], T y[kNumOutputs]) {
    T t[80];
    for (int i = 0; i < 16; ++i) t[i] = x[i];
    for (int i = 16; i < 80; i++)
      t[i] = nldtool::Rotl(t[i - 3] ^ t[i - 8] ^ t[i - 14] ^ t[i - 16], 1,
                           word_size);
    for (int i = 0; i < 64; ++i) y[i] = t[16 + i];
  }
};

constexpr char Sha1::MEXP::kName[];

void Sha1::AddToOptions(cxxopts::Options& options) {
  options.add_options("SHA-1 specific")            //
      ("sha1-mal",                                 //
       "use malicious constants",                  //
       cxxopts::value<int>()->default_value("0"),  //
       "FLAG");
}

Sha1::Sha1(cxxopts::Options& options)
    : Crypto(options["word-size"].as<int>()),
      num_rounds_(options["num-rounds"].as<int>()),
      A_(tA + 5) {
  Step* step;
  bool use_malicious_constants = options["sha1-mal"].as<int>();

  uint64_t k[4] = {0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6};

  for (int i = -5; i < 0; i++) A_[i] = AddConditionWord("I", i, 5 + i, 0);
  for (int i = 0; i < num_rounds_; i++) {
    F[i] = AddConditionWord("F", i, 5 + i * 3 + 0, 0, SUBWORD);
    A_[i] = AddConditionWord("A", i, 5 + i * 3 + 1, 0);
    W[i] = AddConditionWord("W", i, 5 + i * 3 + 1, 1);
  }
  for (int i = num_rounds_; i < 80; i++) {
    W[i] = AddConditionWord("W", i, 5 + i * 3 + 1, 1, SUBWORD);
  }

  Add(new LinearStep<MEXP>(
      word_size_, W[0], W[1], W[2], W[3], W[4], W[5], W[6], W[7], W[8], W[9],
      W[10], W[11], W[12], W[13], W[14], W[15], W[16], W[17], W[18], W[19],
      W[20], W[21], W[22], W[23], W[24], W[25], W[26], W[27], W[28], W[29],
      W[30], W[31], W[32], W[33], W[34], W[35], W[36], W[37], W[38], W[39],
      W[40], W[41], W[42], W[43], W[44], W[45], W[46], W[47], W[48], W[49],
      W[50], W[51], W[52], W[53], W[54], W[55], W[56], W[57], W[58], W[59],
      W[60], W[61], W[62], W[63], W[64], W[65], W[66], W[67], W[68], W[69],
      W[70], W[71], W[72], W[73], W[74], W[75], W[76], W[77], W[78], W[79]));

  ConditionWordPtr K;
  for (int i = 0; i < num_rounds_; i++) {
    ConditionWordPtr CA =
        AddConditionWord("C", i, 5 + i * 3 + 2, 0, CARRYWORD, 3);
    if (i % 20 == 0) {
      if (use_malicious_constants)
        K = AddConditionWord("K", i, 5 + i * 3 + 1, 2);
      else
        K = ConditionWordPtr(new ConditionWord(k[i / 20]));
    }

    // rotated IV:
    ConditionWordPtr E = A_[i - 5]->Rotr(2);
    ConditionWordPtr D = A_[i - 4]->Rotr(2);
    ConditionWordPtr C = A_[i - 3]->Rotr(2);
    ConditionWordPtr B = A_[i - 2];
    ConditionWordPtr A = A_[i - 1]->Rotl(5);

    if (i >= 16)
      Add(new BitsliceStep<XOR<4>>(word_size_, W[i - 3]->Rotr(31),
                                   W[i - 8]->Rotr(31), W[i - 14]->Rotr(31),
                                   W[i - 16]->Rotr(31), W[i]));

    if (i < 20)
      Add(new BitsliceStep<IF>(word_size_, B, C, D, F[i]));
    else if (i < 40)
      Add(new BitsliceStep<XOR<3>>(word_size_, B, C, D, F[i]));
    else if (i < 60)
      Add(new BitsliceStep<MAJ>(word_size_, B, C, D, F[i]));
    else if (i < 80)
      Add(new BitsliceStep<XOR<3>>(word_size_, B, C, D, F[i]));
    Add(new BitsliceStep<ADD<5>>(word_size_, A, F[i], E, K, W[i], CA->Shl(1),
                                 A_[i], CA));

    if (i < 20)
      step = Add(new BitsliceStep<IFADD5>(word_size_, B, C, D, A, E, K, W[i],
                                          CA->Shl(1), F[i], A_[i], CA));
    else if (i < 40)
      step = Add(new BitsliceStep<XOR3ADD5>(word_size_, B, C, D, A, E, K, W[i],
                                            CA->Shl(1), F[i], A_[i], CA));
    else if (i < 60)
      step = Add(new BitsliceStep<MAJADD5>(word_size_, B, C, D, A, E, K, W[i],
                                           CA->Shl(1), F[i], A_[i], CA));
    else  // if (i < 80)
      step = Add(new BitsliceStep<XOR3ADD5>(word_size_, B, C, D, A, E, K, W[i],
                                            CA->Shl(1), F[i], A_[i], CA));

    step->SetProbabilityMethod(CYCLICGRAPH);
    A_[i]->SetStepToComputeProbability(step);
  }
}

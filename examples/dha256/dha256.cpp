#include "dha256/dha256.h"

#include "bitslice_step.h"
#include "carry_step.h"
#include "functions.h"
#include "linear_step.h"

const uint64_t Dha256::K256[64] = {
    0x428A2F98, 0x71374491, 0xB5C0FBCF, 0xE9B5DBA5, 0x3956C25B, 0x59F111F1,
    0x923F82A4, 0xAB1C5ED5, 0xD807AA98, 0x12835B01, 0x243185BE, 0x550C7DC3,
    0x72BE5D74, 0x80DEB1FE, 0x9BDC06A7, 0xC19BF174, 0xE49B69C1, 0xEFBE4786,
    0x0FC19DC6, 0x240CA1CC, 0x2DE92C6F, 0x4A7484AA, 0x5CB0A9DC, 0x76F988DA,
    0x983E5152, 0xA831C66D, 0xB00327C8, 0xBF597FC7, 0xC6E00BF3, 0xD5A79147,
    0x06CA6351, 0x14292967, 0x27B70A85, 0x2E1B2138, 0x4D2C6DFC, 0x53380D13,
    0x650A7354, 0x766A0ABB, 0x81C2C92E, 0x92722C85, 0xA2BFE8A1, 0xA81A664B,
    0xC24B8B70, 0xC76C51A3, 0xD192E819, 0xD6990624, 0xF40E3585, 0x106AA070,
    0x19A4C116, 0x1E376C08, 0x2748774C, 0x34B0BCB5, 0x391C0CB3, 0x4ED8AA4A,
    0x5B9CCA4F, 0x682E6FF3, 0x748F82EE, 0x78A5636F, 0x84C87814, 0x8CC70208,
    0x90BEFFFA, 0xA4506CEB, 0xBEF9A3F7, 0xC67178F2};

class Dha256::P0 : public F {
 public:
  static constexpr char kName[] = "P0";
  static const int kNumInputs = 1;
  static const int kNumOutputs = 1;
  template <class T>
  static void f(int word_size, const T x[kNumInputs], T y[kNumOutputs]) {
    y[0] = x[0] ^ nldtool::Rotl<32>(x[0], 7) ^ nldtool::Rotl<32>(x[0], 22);
  }
};

constexpr char Dha256::P0::kName[];

class Dha256::P1 : public F {
 public:
  static constexpr char kName[] = "P1";
  static const int kNumInputs = 1;
  static const int kNumOutputs = 1;
  template <class T>
  static void f(int word_size, const T x[kNumInputs], T y[kNumOutputs]) {
    y[0] = x[0] ^ nldtool::Rotl<32>(x[0], 13) ^ nldtool::Rotl<32>(x[0], 27);
  }
};

constexpr char Dha256::P1::kName[];

class Dha256::PP0 : public F {
 public:
  static constexpr char kName[] = "P0";
  static const int kNumInputs = 1;
  static const int kNumOutputs = 1;
  template <class T>
  static void f(int word_size, const T x[kNumInputs], T y[kNumOutputs]) {
    y[0] = x[0] ^ nldtool::Rotl<32>(x[0], 11) ^ nldtool::Rotl<32>(x[0], 25);
  }
};

constexpr char Dha256::PP0::kName[];

class Dha256::PP1 : public F {
 public:
  static constexpr char kName[] = "P1";
  static const int kNumInputs = 1;
  static const int kNumOutputs = 1;
  template <class T>
  static void f(int word_size, const T x[kNumInputs], T y[kNumOutputs]) {
    y[0] = x[0] ^ nldtool::Rotl<32>(x[0], 19) ^ nldtool::Rotl<32>(x[0], 29);
  }
};

constexpr char Dha256::PP1::kName[];

void Dha256::AddToOptions(cxxopts::Options& options) {}

Dha256::Dha256(cxxopts::Options& options)
    : Crypto(options["word-size"].as<int>()),
      num_rounds_(options["num-rounds"].as<int>()),
      E_(tE_ + I),
      A_(tA_ + I) {
  for (int i = 0; i < 16; i++)
    W_[i] = AddConditionWord("W", i, I + L * i + 2, 2);
  for (int i = -I; i < 0; i++) E_[i] = AddConditionWord("E", i, I + i, 1);
  for (int i = -I; i < 0; i++) A_[i] = AddConditionWord("A", i, I + i, 0);

  // message expansion
  for (int i = 16; i < num_rounds_; i++) {
    W_[i] = AddConditionWord("W", i, I + L * i + 2, 2, MAINWORD);
    S0_[i] = AddConditionWord("S0", i, I + L * i + 0, 2, SUBWORD);
    S1_[i] = AddConditionWord("S1", i, I + L * i + 1, 2, SUBWORD);

    Add(new BitsliceStep<XOR<3>>(word_size_, W_[i - 1], W_[i - 1]->Rotl(7),
                                 W_[i - 1]->Rotl(22), S0_[i]));
    Add(new LinearStep<P0>(word_size_, W_[i - 1], S0_[i]));
    Add(new BitsliceStep<XOR<3>>(word_size_, W_[i - 15], W_[i - 15]->Rotl(13),
                                 W_[i - 15]->Rotl(27), S1_[i]));
    Add(new LinearStep<P1>(word_size_, W_[i - 15], S1_[i]));
    Add(new CarryStep<ADD<4>>(word_size_, W_[i - 16], W_[i - 9], S0_[i], S1_[i],
                              W_[i]));
  }

  // characteristic update
  for (int i = 0; i < num_rounds_; i++) {
    // rotated IV:
    ConditionWordPtr H = E_[i - 4]->Rotl(17);
    ConditionWordPtr G = E_[i - 3]->Rotl(17);
    ConditionWordPtr F = E_[i - 2];
    ConditionWordPtr E = E_[i - 1];
    ConditionWordPtr D = A_[i - 4]->Rotl(2);
    ConditionWordPtr C = A_[i - 3]->Rotl(2);
    ConditionWordPtr B = A_[i - 2];
    ConditionWordPtr A = A_[i - 1];

    ConditionWordPtr K = ConditionWordPtr(new ConditionWord(K256[i]));
    F0_[i] = AddConditionWord("F0", i, I + L * i + 0, 0, SUBWORD);
    SS0_[i] = AddConditionWord("SS0", i, I + L * i + 1, 0, SUBWORD);
    A_[i] = AddConditionWord("A", i, I + L * i + 2, 0);
    F1_[i] = AddConditionWord("F1", i, I + L * i + 0, 1, SUBWORD);
    SS1_[i] = AddConditionWord("SS1", i, I + L * i + 1, 1, SUBWORD);
    E_[i] = AddConditionWord("E", i, I + L * i + 2, 1);

    Add(new BitsliceStep<IF>(word_size_, G, F, E, F0_[i]));
    Add(new BitsliceStep<XOR<3>>(word_size_, E, E->Rotl(11), E->Rotl(25),
                                 SS0_[i]));
    Add(new LinearStep<PP0>(word_size_, E, SS0_[i]));
    Add(new CarryStep<ADD<5>>(word_size_, SS0_[i], F0_[i], H, K, W_[i], A_[i]));

    Add(new BitsliceStep<MAJ>(word_size_, C, B, A, F1_[i]));
    Add(new BitsliceStep<XOR<3>>(word_size_, A, A->Rotl(19), A->Rotl(29),
                                 SS1_[i]));
    Add(new LinearStep<PP1>(word_size_, A, SS1_[i]));
    Add(new CarryStep<ADD<5>>(word_size_, SS1_[i], F1_[i], D, K, W_[i], E_[i]));
  }
}

bool Dha256::Callback(std::string function, Characteristic& characteristic,
                      std::mt19937& rng, Logfile& logfile) {
  return Crypto::Callback(function, characteristic, rng, logfile);
}

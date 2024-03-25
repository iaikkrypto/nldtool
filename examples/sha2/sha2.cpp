#include "sha2/sha2.h"

#include "bitslice_step.h"
#include "functions.h"
#include "linear_step.h"

const uint64_t Sha2::K512[80] = {
    0x428a2f98d728ae22ull, 0x7137449123ef65cdull, 0xb5c0fbcfec4d3b2full,
    0xe9b5dba58189dbbcull, 0x3956c25bf348b538ull, 0x59f111f1b605d019ull,
    0x923f82a4af194f9bull, 0xab1c5ed5da6d8118ull, 0xd807aa98a3030242ull,
    0x12835b0145706fbeull, 0x243185be4ee4b28cull, 0x550c7dc3d5ffb4e2ull,
    0x72be5d74f27b896full, 0x80deb1fe3b1696b1ull, 0x9bdc06a725c71235ull,
    0xc19bf174cf692694ull, 0xe49b69c19ef14ad2ull, 0xefbe4786384f25e3ull,
    0x0fc19dc68b8cd5b5ull, 0x240ca1cc77ac9c65ull, 0x2de92c6f592b0275ull,
    0x4a7484aa6ea6e483ull, 0x5cb0a9dcbd41fbd4ull, 0x76f988da831153b5ull,
    0x983e5152ee66dfabull, 0xa831c66d2db43210ull, 0xb00327c898fb213full,
    0xbf597fc7beef0ee4ull, 0xc6e00bf33da88fc2ull, 0xd5a79147930aa725ull,
    0x06ca6351e003826full, 0x142929670a0e6e70ull, 0x27b70a8546d22ffcull,
    0x2e1b21385c26c926ull, 0x4d2c6dfc5ac42aedull, 0x53380d139d95b3dfull,
    0x650a73548baf63deull, 0x766a0abb3c77b2a8ull, 0x81c2c92e47edaee6ull,
    0x92722c851482353bull, 0xa2bfe8a14cf10364ull, 0xa81a664bbc423001ull,
    0xc24b8b70d0f89791ull, 0xc76c51a30654be30ull, 0xd192e819d6ef5218ull,
    0xd69906245565a910ull, 0xf40e35855771202aull, 0x106aa07032bbd1b8ull,
    0x19a4c116b8d2d0c8ull, 0x1e376c085141ab53ull, 0x2748774cdf8eeb99ull,
    0x34b0bcb5e19b48a8ull, 0x391c0cb3c5c95a63ull, 0x4ed8aa4ae3418acbull,
    0x5b9cca4f7763e373ull, 0x682e6ff3d6b2b8a3ull, 0x748f82ee5defb2fcull,
    0x78a5636f43172f60ull, 0x84c87814a1f0ab72ull, 0x8cc702081a6439ecull,
    0x90befffa23631e28ull, 0xa4506cebde82bde9ull, 0xbef9a3f7b2c67915ull,
    0xc67178f2e372532bull, 0xca273eceea26619cull, 0xd186b8c721c0c207ull,
    0xeada7dd6cde0eb1eull, 0xf57d4f7fee6ed178ull, 0x06f067aa72176fbaull,
    0x0a637dc5a2c898a6ull, 0x113f9804bef90daeull, 0x1b710b35131c471bull,
    0x28db77f523047d84ull, 0x32caab7b40c72493ull, 0x3c9ebe0a15c9bebcull,
    0x431d67c49c100d4cull, 0x4cc5d4becb3e42b6ull, 0x597f299cfc657e2aull,
    0x5fcb6fab3ad6faecull, 0x6c44198c4a475817ull};

constexpr int Sha2::Rot256[12];
constexpr int Sha2::Rot512[12];

const int Sha2::H = 4;
const int Sha2::L = 5;
const int Sha2::F = 12;

void Sha2::AddToOptions(cxxopts::Options& options) {
  options.add_options("SHA-2 specific")            //
      ("sha2-lin",                                 //
       "use linear layer",                         //
       cxxopts::value<int>()->default_value("0"),  //
       "FLAG");
}

Sha2::Sha2(cxxopts::Options& options)
    : Crypto(options["word-size"].as<int>()),
      S(word_size_ == 32 ? Rot256 : Rot512),
      num_rounds_(options["num-rounds"].as<int>()),
      use_linear_layer_(options["sha2-lin"].as<int>()),
      E(tE + H),
      A(tA + H) {
  s = 0;
  h = s % 2;
  v = s / 2;
  V = H + num_rounds_ * L + 8;
  m = std::min(16, num_rounds_);

  InitWords();
  InitSub();
}

template <int R0, int R1, int S2>
class Sha2::sigma : public F {
 public:
  static constexpr char kName[] = "sigma";
  static const int kNumInputs = 1;
  static const int kNumOutputs = 1;
  template <class T>
  static void f(int word_size, const T x[kNumInputs], T y[kNumOutputs]) {
    y[0] = nldtool::Rotr(x[0], R0, word_size) ^
           nldtool::Rotr(x[0], R1, word_size) ^
           nldtool::Shr(x[0], S2, word_size);
  }
};

template <int R0, int R1, int R2>
class Sha2::Sigma : public F {
 public:
  static constexpr char kName[] = "SIGMA";
  static const int kNumInputs = 1;
  static const int kNumOutputs = 1;
  template <class T>
  static void f(int word_size, const T x[kNumInputs], T y[kNumOutputs]) {
    y[0] = nldtool::Rotr(x[0], R0, word_size) ^
           nldtool::Rotr(x[0], R1, word_size) ^
           nldtool::Rotr(x[0], R2, word_size);
  }
};

class Sha2::SADD : public F {
 public:
  static constexpr char kName[] = "SADD";
  static const int kNumInputs = 5;
  static const int kNumOutputs = 2;
  static const int kPrevState = 4;
  static const int kNextState = 6;
  static const int kStateSize = 16;
  static constexpr int Symmetry(int i) { return (i < 3) ? 0 : i; }
  static constexpr int Bitsize(int i) { return (i == 4 || i == 6) ? 2 : 1; }
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    const int ci = kNumInputs - 1;
    const int s = 0;
    const int co = 1;
    const T cin = {(x[ci].first == 3) ? uint8_t(-1) : x[ci].first,
                   (x[ci].second == 3) ? uint8_t(-1) : x[ci].second};
    y[s] = x[0] + x[1] + x[2] - x[3] + cin;
    y[co] = (y[s] >> 1) & 3;
    y[s] &= 1;
  }
};

constexpr char Sha2::SADD::kName[];

void Sha2::InitWords() {
  for (int i = 0; i < m; i++)
    W[i] = AddConditionWord("W", i, V * v + H + i * L + 2, 2 + h * 6);
  for (int i = -H; i < 0; i++)
    E[i] = AddConditionWord("E", i, V * v + H + i, 1 + h * 6);
  for (int i = -H; i < 0; i++)
    A[i] = AddConditionWord("A", i, V * v + H + i, 0 + h * 6);
  for (int i = 0; i < num_rounds_; i++) {
    if (i >= 16) {
      ms0[i] =
          AddConditionWord("s0", i, V * v + H + i * L + 0, 2 + h * 6, SUBWORD);
      ms1[i] =
          AddConditionWord("s1", i, V * v + H + i * L + 1, 2 + h * 6, SUBWORD);
      W[i] = AddConditionWord("W", i, V * v + H + i * L + 2, 2 + h * 6);
      CW[i] = AddConditionWord("CW", i, V * v + H + i * L + 3, 2 + h * 6,
                               CARRYWORD, 2);
    }
    KW[i] = ConditionWordPtr(
        new ConditionWord(K512[i] >> (64 - word_size_), word_size_));
    mIF[i] =
        AddConditionWord("IF", i, V * v + H + i * L + 0, 1 + h * 6, SUBWORD);
    mS1[i] =
        AddConditionWord("S1", i, V * v + H + i * L + 1, 1 + h * 6, SUBWORD);
    E[i] = AddConditionWord("E", i, V * v + H + i * L + 2, 1 + h * 6);
    CE[i] = AddConditionWord("CE", i, V * v + H + i * L + 3, 1 + h * 6,
                             CARRYWORD, 3);
    mMJ[i] =
        AddConditionWord("MJ", i, V * v + H + i * L + 0, 0 + h * 6, SUBWORD);
    mS0[i] =
        AddConditionWord("S0", i, V * v + H + i * L + 1, 0 + h * 6, SUBWORD);
    A[i] = AddConditionWord("A", i, V * v + H + i * L + 2, 0 + h * 6);
    CA[i] = AddConditionWord("CA", i, V * v + H + i * L + 3, 0 + h * 6,
                             CARRYWORD, 2);
  }
}

void Sha2::InitSub() {
  for (int i = 0; i < num_rounds_; i++) {
    Step* step = 0;
    if (i >= 16) {
      if (use_linear_layer_) {
        if (word_size_ == 32) {
          step = Add(new LinearStep<sigma<Rot256[0], Rot256[1], Rot256[2]>>(
              word_size_, W[i - 15], ms0[i]));
          ms0[i]->SetStepToComputeProbability(step);
          step = Add(new LinearStep<sigma<Rot256[3], Rot256[4], Rot256[5]>>(
              word_size_, W[i - 2], ms1[i]));
          ms1[i]->SetStepToComputeProbability(step);
        }
        if (word_size_ == 64) {
          step = Add(new LinearStep<sigma<Rot512[0], Rot512[1], Rot512[2]>>(
              word_size_, W[i - 15], ms0[i]));
          ms0[i]->SetStepToComputeProbability(step);
          step = Add(new LinearStep<sigma<Rot512[3], Rot512[4], Rot512[5]>>(
              word_size_, W[i - 2], ms1[i]));
          ms1[i]->SetStepToComputeProbability(step);
        }
      } else {  // use bitslice step
        step = Add(new BitsliceStep<XOR<3>>(word_size_, W[i - 15]->Rotr(S[0]),
                                            W[i - 15]->Rotr(S[1]),
                                            W[i - 15]->Shr(S[2]), ms0[i]));
        ms0[i]->SetStepToComputeProbability(step);
        step = Add(new BitsliceStep<XOR<3>>(word_size_, W[i - 2]->Rotr(S[3]),
                                            W[i - 2]->Rotr(S[4]),
                                            W[i - 2]->Shr(S[5]), ms1[i]));
        ms1[i]->SetStepToComputeProbability(step);
      }

      step =
          Add(new BitsliceStep<ADD<4>>(word_size_, ms1[i], ms0[i], W[i - 7],
                                       W[i - 16], CW[i]->Shl(1), W[i], CW[i]));
      step->SetProbabilityMethod(CYCLICGRAPH);
      W[i]->SetStepToComputeProbability(step);
    }
    // IF step
    step = Add(
        new BitsliceStep<IF>(word_size_, E[i - 1], E[i - 2], E[i - 3], mIF[i]));
    mIF[i]->SetStepToComputeProbability(step);

    // Sigma 1 (linear)
    if (use_linear_layer_) {
      if (word_size_ == 32) {
        step = Add(new LinearStep<Sigma<Rot256[6], Rot256[7], Rot256[8]>>(
            word_size_, E[i - 1], mS1[i]));
        mS1[i]->SetStepToComputeProbability(step);
      }
      if (word_size_ == 64) {
        step = Add(new LinearStep<Sigma<Rot512[6], Rot512[7], Rot512[8]>>(
            word_size_, E[i - 1], mS1[i]));
        mS1[i]->SetStepToComputeProbability(step);
      }
    } else {  // bitsliced
      step = Add(new BitsliceStep<XOR<3>>(word_size_, E[i - 1]->Rotr(S[6]),
                                          E[i - 1]->Rotr(S[7]),
                                          E[i - 1]->Rotr(S[8]), mS1[i]));
      mS1[i]->SetStepToComputeProbability(step);
    }

    // adding IF, Sigma1, W, const, old A and old E
    step = Add(new BitsliceStep<ADD<6>>(word_size_, mS1[i], mIF[i], W[i],
                                        A[i - 4], E[i - 4], KW[i],
                                        CE[i]->Shl(1), E[i], CE[i]));
    step->SetProbabilityMethod(CYCLICGRAPH);
    E[i]->SetStepToComputeProbability(step);

    // MAJ step
    step = Add(new BitsliceStep<MAJ>(word_size_, A[i - 1], A[i - 2], A[i - 3],
                                     mMJ[i]));
    mMJ[i]->SetStepToComputeProbability(step);

    // Sigma 0 (linear)
    if (use_linear_layer_) {
      if (word_size_ == 32) {
        step = Add(new LinearStep<Sigma<Rot256[9], Rot256[10], Rot256[11]>>(
            word_size_, A[i - 1], mS0[i]));
        mS0[i]->SetStepToComputeProbability(step);
      }
      if (word_size_ == 64) {
        step = Add(new LinearStep<Sigma<Rot512[9], Rot512[10], Rot512[11]>>(
            word_size_, A[i - 1], mS0[i]));
        mS0[i]->SetStepToComputeProbability(step);
      }
    } else {
      step = Add(new BitsliceStep<XOR<3>>(word_size_, A[i - 1]->Rotr(S[9]),
                                          A[i - 1]->Rotr(S[10]),
                                          A[i - 1]->Rotr(S[11]), mS0[i]));
      mS0[i]->SetStepToComputeProbability(step);
    }

    // adding S0, MAJ, E and old A
    step = Add(new BitsliceStep<SADD>(word_size_, mS0[i], mMJ[i], E[i],
                                      A[i - 4], CA[i]->Shl(1), A[i], CA[i]));
    step->SetProbabilityMethod(CYCLICGRAPH);
    A[i]->SetStepToComputeProbability(step);
  }
}

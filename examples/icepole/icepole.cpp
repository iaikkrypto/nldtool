#include "icepole/icepole.h"

#include <limits>

#include "bitslice_step.h"
#include "functions.h"
#include "linear_step.h"
#include "logfile.h"

constexpr uint64_t Icepole::ROUND_CONSTANTS[12];
constexpr int Icepole::ROTATION_VALUES[4][5];

void Icepole::AddToOptions(cxxopts::Options& options) {
  options.add_options("Icepole specific")          //
      ("icepole-part",                             //
       "search part",                              //
       cxxopts::value<int>()->default_value("1"),  //
       "N")                                        //
      ("icepole-ls",                               //
       "linear flags",                             //
       cxxopts::value<int>()->default_value("0"),  //
       "FLAG");
}

Icepole::Icepole(cxxopts::Options& options)
    : Crypto(options["word-size"].as<int>()),
      num_rounds_(options["num-rounds"].as<int>()),
      part_(options["icepole-part"].as<int>()),
      linear_flags_(options["icepole-ls"].as<int>()),
      best_prob_(-std::numeric_limits<float>::infinity()),
      best_char_(std::numeric_limits<uint64_t>().max()) {
  std::string char_index[] = {"a", "b", "c", "d", "e"};

  if (word_size_ == 64 && num_rounds_ <= ICEPOLE_MAXROUNDS) {
    //------------Creating words--------------
    for (int r = 0; r < num_rounds_; ++r) {
      for (int x = 0; x < 4; ++x) {
        for (int y = 0; y < 5; ++y) {
          S[r][x][y] =
              AddConditionWord("S" + char_index[x] + "_" + char_index[y], r,
                               y + x * 5 + r * 40, 0);
          A[r][x][y] =
              AddConditionWord("A" + char_index[x] + "_" + char_index[y], r,
                               y + x * 5 + r * 40 + 20, 0, SUBWORD);
        }
        if (part_ == 1)
          ACTIVE[r][x] = AddConditionWord("ACT_" + char_index[x], r + 1,
                                          4 + x * 5 + (r + 1) * 40, 1);
      }
    }

    for (int x = 0; x < 4; ++x)
      for (int y = 0; y < 5; ++y) {
        S[num_rounds_][x][y] =
            AddConditionWord("S" + char_index[x] + "_" + char_index[y],
                             num_rounds_, y + x * 5 + num_rounds_ * 40, 0);
      }
    for (int r = 0; r < num_rounds_; ++r)
      C[r] = AddConditionWord("C", r, 1000 + r, 0, SUBWORD);

    for (int r = 0; r < num_rounds_; ++r) {
      if (linear_flags_ == 1) mue_rho_lin(r);
      mue_rho(r);
      pi(r);
      psi(r);
      kappa(r);
    }
  }
}

bool Icepole::Callback(std::string function, Characteristic& characteristic,
                       std::mt19937& rng, Logfile& logfile) {
  if (function.empty())
    return true;
  else if (function.compare("found") == 0) {
    if (part_ == 2) {
      float prob = characteristic.GetProbabilitySum();
      if (prob > best_prob_) {
        logfile << "found new characteristic" << std::endl;
        logfile << "prob: " << prob << std::endl;
        characteristic.WriteCharacteristic(logfile);
        best_prob_ = prob;
      }
    }
    if (part_ == 1) {
      uint64_t cost = 0;
      uint64_t trunc_diff = 0;
      uint64_t trunc_cost = 0;
      for (int r = 1; r <= num_rounds_; r++) {
        trunc_diff = characteristic.GetConditionWordMask(
            [](BitCondition bc) { return bc == BitCondition("1"); },
            characteristic.GetCrypto()->GetConditionWordIndex("ACT_a", r));
        trunc_cost = nldtool::HammingWeight(trunc_diff);
        cost += trunc_cost;
        trunc_diff = characteristic.GetConditionWordMask(
            [](BitCondition bc) { return bc == BitCondition("1"); },
            characteristic.GetCrypto()->GetConditionWordIndex("ACT_b", r));
        trunc_cost = nldtool::HammingWeight(trunc_diff);
        cost += trunc_cost;
        trunc_diff = characteristic.GetConditionWordMask(
            [](BitCondition bc) { return bc == BitCondition("1"); },
            characteristic.GetCrypto()->GetConditionWordIndex("ACT_c", r));
        trunc_cost = nldtool::HammingWeight(trunc_diff);
        cost += trunc_cost;
        trunc_diff = characteristic.GetConditionWordMask(
            [](BitCondition bc) { return bc == BitCondition("1"); },
            characteristic.GetCrypto()->GetConditionWordIndex("ACT_d", r));
        trunc_cost = nldtool::HammingWeight(trunc_diff);
        cost += trunc_cost;
      }
      if (cost < best_char_) {
        logfile << "found new characteristic" << std::endl;
        logfile << "active S-boxes: " << cost << std::endl;
        characteristic.WriteCharacteristic(logfile);
        best_char_ = cost;
      }
    }
    return true;
  }
  assert(false);
  return false;
}

void Icepole::mue_rho(int rb) {
  Step* step;
  step = Add(new BitsliceStep<XOR<4>>(
      word_size_, S[rb][0][3], S[rb][1][4], S[rb][2][4], S[rb][3][4],
      A[rb][0][4]->Rotr(ROTATION_VALUES[0][4])));
  if (part_ == 2) A[rb][0][4]->SetStepToComputeProbability(step);

  step = Add(new BitsliceStep<XOR<4>>(
      word_size_, S[rb][0][2], S[rb][1][3], S[rb][2][3], S[rb][3][3],
      A[rb][0][3]->Rotr(ROTATION_VALUES[0][3])));
  if (part_ == 2) A[rb][0][3]->SetStepToComputeProbability(step);

  step = Add(new BitsliceStep<XOR<5>>(
      word_size_, S[rb][0][4], S[rb][0][1], S[rb][1][2], S[rb][2][2],
      S[rb][3][2], A[rb][0][2]->Rotr(ROTATION_VALUES[0][2])));
  if (part_ == 2) A[rb][0][2]->SetStepToComputeProbability(step);

  step = Add(new BitsliceStep<XOR<4>>(
      word_size_, S[rb][0][0], S[rb][1][1], S[rb][2][1], S[rb][3][1],
      A[rb][0][1]->Rotr(ROTATION_VALUES[0][1])));
  if (part_ == 2) A[rb][0][1]->SetStepToComputeProbability(step);

  step = Add(new BitsliceStep<XOR<4>>(
      word_size_, S[rb][0][4], S[rb][1][0], S[rb][2][0], S[rb][3][0],
      A[rb][0][0]->Rotr(ROTATION_VALUES[0][0])));
  if (part_ == 2) A[rb][0][0]->SetStepToComputeProbability(step);

  step = Add(new BitsliceStep<XOR<4>>(
      word_size_, S[rb][0][4], S[rb][1][4], S[rb][2][0], S[rb][3][3],
      A[rb][1][4]->Rotr(ROTATION_VALUES[1][4])));
  if (part_ == 2) A[rb][1][4]->SetStepToComputeProbability(step);

  step = Add(new BitsliceStep<XOR<4>>(
      word_size_, S[rb][0][3], S[rb][1][3], S[rb][2][4], S[rb][3][2],
      A[rb][1][3]->Rotr(ROTATION_VALUES[1][3])));
  if (part_ == 2) A[rb][1][3]->SetStepToComputeProbability(step);

  step = Add(new BitsliceStep<XOR<5>>(
      word_size_, S[rb][0][2], S[rb][1][2], S[rb][2][3], S[rb][3][4],
      S[rb][3][1], A[rb][1][2]->Rotr(ROTATION_VALUES[1][2])));
  if (part_ == 2) A[rb][1][2]->SetStepToComputeProbability(step);

  step = Add(new BitsliceStep<XOR<5>>(
      word_size_, S[rb][0][1], S[rb][1][1], S[rb][2][2], S[rb][2][0],
      S[rb][3][0], A[rb][1][1]->Rotr(ROTATION_VALUES[1][1])));
  if (part_ == 2) A[rb][1][1]->SetStepToComputeProbability(step);

  step = Add(new BitsliceStep<XOR<4>>(
      word_size_, S[rb][0][0], S[rb][1][0], S[rb][2][1], S[rb][3][4],
      A[rb][1][0]->Rotr(ROTATION_VALUES[1][0])));
  if (part_ == 2) A[rb][1][0]->SetStepToComputeProbability(step);

  step = Add(new BitsliceStep<XOR<4>>(
      word_size_, S[rb][0][4], S[rb][1][3], S[rb][2][4], S[rb][3][0],
      A[rb][2][4]->Rotr(ROTATION_VALUES[2][4])));
  if (part_ == 2) A[rb][2][4]->SetStepToComputeProbability(step);

  step = Add(new BitsliceStep<XOR<4>>(
      word_size_, S[rb][0][3], S[rb][1][2], S[rb][2][3], S[rb][3][4],
      A[rb][2][3]->Rotr(ROTATION_VALUES[2][3])));
  if (part_ == 2) A[rb][2][3]->SetStepToComputeProbability(step);

  step = Add(new BitsliceStep<XOR<5>>(
      word_size_, S[rb][0][2], S[rb][1][4], S[rb][1][1], S[rb][2][2],
      S[rb][3][3], A[rb][2][2]->Rotr(ROTATION_VALUES[2][2])));
  if (part_ == 2) A[rb][2][2]->SetStepToComputeProbability(step);

  step = Add(new BitsliceStep<XOR<5>>(
      word_size_, S[rb][0][1], S[rb][1][0], S[rb][2][1], S[rb][3][2],
      S[rb][3][0], A[rb][2][1]->Rotr(ROTATION_VALUES[2][1])));
  if (part_ == 2) A[rb][2][1]->SetStepToComputeProbability(step);

  step = Add(new BitsliceStep<XOR<4>>(
      word_size_, S[rb][0][0], S[rb][1][4], S[rb][2][0], S[rb][3][1],
      A[rb][2][0]->Rotr(ROTATION_VALUES[2][0])));
  if (part_ == 2) A[rb][2][0]->SetStepToComputeProbability(step);

  step = Add(new BitsliceStep<XOR<4>>(
      word_size_, S[rb][0][4], S[rb][1][0], S[rb][2][3], S[rb][3][4],
      A[rb][3][4]->Rotr(ROTATION_VALUES[3][4])));
  if (part_ == 2) A[rb][3][4]->SetStepToComputeProbability(step);

  step = Add(new BitsliceStep<XOR<4>>(
      word_size_, S[rb][0][3], S[rb][1][4], S[rb][2][2], S[rb][3][3],
      A[rb][3][3]->Rotr(ROTATION_VALUES[3][3])));
  if (part_ == 2) A[rb][3][3]->SetStepToComputeProbability(step);

  step = Add(new BitsliceStep<XOR<5>>(
      word_size_, S[rb][0][2], S[rb][1][3], S[rb][2][4], S[rb][2][1],
      S[rb][3][2], A[rb][3][2]->Rotr(ROTATION_VALUES[3][2])));
  if (part_ == 2) A[rb][3][2]->SetStepToComputeProbability(step);

  step = Add(new BitsliceStep<XOR<5>>(
      word_size_, S[rb][0][1], S[rb][1][2], S[rb][1][0], S[rb][2][0],
      S[rb][3][1], A[rb][3][1]->Rotr(ROTATION_VALUES[3][1])));
  if (part_ == 2) A[rb][3][1]->SetStepToComputeProbability(step);

  step = Add(new BitsliceStep<XOR<4>>(
      word_size_, S[rb][0][0], S[rb][1][1], S[rb][2][4], S[rb][3][0],
      A[rb][3][0]->Rotr(ROTATION_VALUES[3][0])));
  if (part_ == 2) A[rb][3][0]->SetStepToComputeProbability(step);
}

void Icepole::pi(int rb) {
  for (int x = 0; x < 4; ++x)
    for (int y = 0; y < 5; ++y)
      B[rb][(x + y) % 4][(((x + y) % 4) + y + 1) % 5] = A[rb][x][y];
}

void Icepole::psi(int rb) {
  int re = rb + 1;
  Step* step;
  step = Add(new BitsliceStep<Icepole::SBOX>(
      word_size_, B[rb][0][0], B[rb][0][1], B[rb][0][2], B[rb][0][3],
      B[rb][0][4], C[rb], S[re][0][1], S[re][0][2], S[re][0][3], S[re][0][4]));
  if (part_ == 2) S[re][0][4]->SetStepToComputeProbability(step);

  if (part_ == 1) {
    step = Add(new BitsliceStep<Icepole::isActive>(
        word_size_, C[rb], S[re][0][1], S[re][0][2], S[re][0][3], S[re][0][4],
        ACTIVE[rb][0]));
    step->SetProbabilityMethod(3);
    ACTIVE[rb][0]->SetStepToComputeProbability(step);
  }

  for (int x = 1; x < 4; ++x) {
    step = Add(new BitsliceStep<Icepole::SBOX>(
        word_size_, B[rb][x][0], B[rb][x][1], B[rb][x][2], B[rb][x][3],
        B[rb][x][4], S[re][x][0], S[re][x][1], S[re][x][2], S[re][x][3],
        S[re][x][4]));
    if (part_ == 2) S[re][x][4]->SetStepToComputeProbability(step);

    if (part_ == 1) {
      step = Add(new BitsliceStep<Icepole::isActive>(
          word_size_, S[re][x][0], S[re][x][1], S[re][x][2], S[re][x][3],
          S[re][x][4], ACTIVE[rb][x]));
      step->SetProbabilityMethod(3);
      // if(rb != num_rounds_ -1 || x < 2)
      // if(rb != num_rounds_ -1 || x < 2)
      ACTIVE[rb][x]->SetStepToComputeProbability(step);
    }
  }
}

void Icepole::kappa(int rb) {
  Step* step;
  step = Add(new BitsliceStep<XOR<2>>(
      word_size_,
      ConditionWordPtr(new ConditionWord(ROUND_CONSTANTS[rb], word_size_)),
      C[rb], S[rb + 1][0][0]));

  if (part_ == 2) S[rb + 1][0][0]->SetStepToComputeProbability(step);
}

class Icepole::SBOX : public F {
 public:
  static constexpr char kName[] = "SBOX";
  static const int kNumInputs = 5;
  static const int kNumOutputs = 5;
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    T c = (x[0] & x[1] & x[2] & x[3] & x[4]) ^
          ((!x[0]) & (!x[1]) & (!x[2]) & (!x[3]) & (!x[4]));
    y[0] = x[0] ^ ((!x[1]) & x[2]) ^ c;
    y[1] = x[1] ^ ((!x[2]) & x[3]) ^ c;
    y[2] = x[2] ^ ((!x[3]) & x[4]) ^ c;
    y[3] = x[3] ^ ((!x[4]) & x[0]) ^ c;
    y[4] = x[4] ^ ((!x[0]) & x[1]) ^ c;
  }
};

constexpr char Icepole::SBOX::kName[];

void Icepole::mue_rho_lin(int rb) {
  Add(new LinearStep<Icepole::Linear_Layer>(
      word_size_, {S[rb][0][0],
                   S[rb][0][1],
                   S[rb][0][2],
                   S[rb][0][3],
                   S[rb][0][4],
                   S[rb][1][0],
                   S[rb][1][1],
                   S[rb][1][2],
                   S[rb][1][3],
                   S[rb][1][4],
                   S[rb][2][0],
                   S[rb][2][1],
                   S[rb][2][2],
                   S[rb][2][3],
                   S[rb][2][4],
                   S[rb][3][0],
                   S[rb][3][1],
                   S[rb][3][2],
                   S[rb][3][3],
                   S[rb][3][4],
                   A[rb][0][0]->Rotr(ROTATION_VALUES[0][0]),
                   A[rb][0][1]->Rotr(ROTATION_VALUES[0][1]),
                   A[rb][0][2]->Rotr(ROTATION_VALUES[0][2]),
                   A[rb][0][3]->Rotr(ROTATION_VALUES[0][3]),
                   A[rb][0][4]->Rotr(ROTATION_VALUES[0][4]),
                   A[rb][1][0]->Rotr(ROTATION_VALUES[1][0]),
                   A[rb][1][1]->Rotr(ROTATION_VALUES[1][1]),
                   A[rb][1][2]->Rotr(ROTATION_VALUES[1][2]),
                   A[rb][1][3]->Rotr(ROTATION_VALUES[1][3]),
                   A[rb][1][4]->Rotr(ROTATION_VALUES[1][4]),
                   A[rb][2][0]->Rotr(ROTATION_VALUES[2][0]),
                   A[rb][2][1]->Rotr(ROTATION_VALUES[2][1]),
                   A[rb][2][2]->Rotr(ROTATION_VALUES[2][2]),
                   A[rb][2][3]->Rotr(ROTATION_VALUES[2][3]),
                   A[rb][2][4]->Rotr(ROTATION_VALUES[2][4]),
                   A[rb][3][0]->Rotr(ROTATION_VALUES[3][0]),
                   A[rb][3][1]->Rotr(ROTATION_VALUES[3][1]),
                   A[rb][3][2]->Rotr(ROTATION_VALUES[3][2]),
                   A[rb][3][3]->Rotr(ROTATION_VALUES[3][3]),
                   A[rb][3][4]->Rotr(ROTATION_VALUES[3][4])}));
}

class Icepole::isActive : public F {
 public:
  static constexpr char kName[] = "ISA";
  static const int kNumInputs = 5;
  static const int kNumOutputs = 1;
  static constexpr int Symmetry(int i) { return i == 5; }
  static void f(const Pair x[kNumInputs], Pair y[kNumOutputs]) {
    y[0].first = y[0].second =
        (x[0].first ^ x[0].second) | (x[1].first ^ x[1].second) |
        (x[2].first ^ x[2].second) | (x[3].first ^ x[3].second) |
        (x[4].first ^ x[4].second);
  }
};

constexpr char Icepole::isActive::kName[];

class Icepole::colActive : public F {
 public:
  static constexpr char kName[] = "ISA";
  static const int kNumInputs = 1;
  static const int kNumOutputs = 4;
  static constexpr int Symmetry(int i) { return i != 0; }
  static void f(const Pair x[kNumInputs], Pair y[kNumOutputs]) {
    y[0].first = y[0].second = y[1].first = y[1].second = y[2].first =
        y[2].second = y[3].first = y[3].second = x[0].first;
  }
};

constexpr char Icepole::colActive::kName[];

class Icepole::Linear_Layer : public F {
 public:
  static constexpr char kName[] = "LinearMue";
  static const int kNumInputs = 20;
  static const int kNumOutputs = 20;
  template <class T>
  static void f(int word_size, const T x[kNumInputs], T y[kNumOutputs]) {
    y[4] = x[3] ^ x[9] ^ x[14] ^ x[19];
    y[3] = x[2] ^ x[8] ^ x[13] ^ x[18];
    y[2] = x[4] ^ x[1] ^ x[7] ^ x[12] ^ x[17];
    y[1] = x[0] ^ x[6] ^ x[11] ^ x[16];
    y[0] = x[4] ^ x[5] ^ x[10] ^ x[15];

    y[9] = x[4] ^ x[9] ^ x[10] ^ x[18];
    y[8] = x[3] ^ x[8] ^ x[14] ^ x[17];
    y[7] = x[2] ^ x[7] ^ x[13] ^ x[19] ^ x[16];
    y[6] = x[1] ^ x[6] ^ x[12] ^ x[10] ^ x[15];
    y[5] = x[0] ^ x[5] ^ x[11] ^ x[19];

    y[14] = x[4] ^ x[8] ^ x[14] ^ x[15];
    y[13] = x[3] ^ x[7] ^ x[13] ^ x[19];
    y[12] = x[2] ^ x[9] ^ x[6] ^ x[12] ^ x[18];
    y[11] = x[1] ^ x[5] ^ x[11] ^ x[17] ^ x[15];
    y[10] = x[0] ^ x[9] ^ x[10] ^ x[16];

    y[19] = x[4] ^ x[5] ^ x[13] ^ x[19];
    y[18] = x[3] ^ x[9] ^ x[12] ^ x[18];
    y[17] = x[2] ^ x[8] ^ x[14] ^ x[11] ^ x[17];
    y[16] = x[1] ^ x[7] ^ x[5] ^ x[10] ^ x[16];
    y[15] = x[0] ^ x[6] ^ x[14] ^ x[15];
  }
};

constexpr char Icepole::Linear_Layer::kName[];

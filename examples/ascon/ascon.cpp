#include "ascon/ascon.h"

#include <type_traits>

#include "bitslice_step.h"
#include "functions.h"
#include "linear_step.h"
#include "logfile.h"

constexpr int Ascon::RotVal[10];
constexpr uint64_t Ascon::RoundConstant[12];
constexpr uint8_t Ascon::LUT[];

template <int R0, int R1, int R2>
class Ascon::Sigma : public F {
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

template <int R0, int R1, int R2>
constexpr char Ascon::Sigma<R0, R1, R2>::kName[];

class Ascon::Sbox : public F {
 public:
  static constexpr char kName[] = "SBOX";
  static const int kNumInputs = 5;
  static const int kNumOutputs = 5;
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    T t0 = x[0] ^ x[4];
    T t1 = x[1];
    T t2 = x[2] ^ x[1];
    T t3 = x[3];
    T t4 = x[4] ^ x[3];
    y[0] = t0 ^ (t2 & !t1);
    y[1] = t1 ^ (t3 & !t2);
    y[2] = t2 ^ (t4 & !t3);
    y[3] = t3 ^ (t0 & !t4);
    y[4] = t4 ^ (t1 & !t0);
    y[1] ^= y[0];
    y[0] ^= y[4];
    y[3] ^= y[2];
    y[2] = !y[2];
  }
};

class Ascon::isActive : public F {
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

constexpr char Ascon::Sbox::kName[];
constexpr char Ascon::isActive::kName[];

void Ascon::AddToOptions(cxxopts::Options& options) {}

Ascon::Ascon(cxxopts::Options& options)
    : Crypto(options["word-size"].as<int>()),
      num_rounds_(options["num-rounds"].as<int>()),
      start_round_(options["start-round"].as<int>()),
      best_char_(std::numeric_limits<uint64_t>().max()) {
  for (int i = 0; i <= num_rounds_; i++) {
    for (int c = 0; c < 5; c++) {
      A[i][c] = AddConditionWord({'x', char('0' + c)}, i, i * 11 + c, 0);
    }
    S[i] = AddConditionWord("S", i, i * 11 + 10, 0);
  }

  for (int i = 0; i < num_rounds_; i++) {
    for (int c = 0; c < 5; c++) {
      B[i][c] =
          AddConditionWord({'y', char('0' + c)}, i, i * 11 + 5 + c, 0, SUBWORD);
    }
    T_A2[i] = AddConditionWord("A2", i, i * 11 + 2, 1, SUBWORD);
  }

  Add(new BitsliceStep<Ascon::isActive>(word_size_, A[0][0], A[0][1], A[0][2],
                                        A[0][3], A[0][4], S[0]));

  for (int i = 1; i <= num_rounds_; i++) {
    ConditionWordPtr k(
        new ConditionWord(RoundConstant[start_round_ + i - 1], word_size_));
    Step* step =
        Add(new BitsliceStep<XOR<2>>(word_size_, A[i - 1][2], k, T_A2[i - 1]));
    T_A2[i - 1]->SetStepToComputeProbability(step);

    step = Add(new BitsliceStep<SBOX<5, 5, LUT>>(
        word_size_, A[i - 1][0], A[i - 1][1], T_A2[i - 1], A[i - 1][3],
        A[i - 1][4], B[i - 1][0], B[i - 1][1], B[i - 1][2], B[i - 1][3],
        B[i - 1][4]));
    B[i - 1][0]->SetStepToComputeProbability(step);

    step = Add(new LinearStep<Ascon::Sigma<0, RotVal[0], RotVal[1]>>(
        word_size_, B[i - 1][0], A[i][0]));
    A[i][0]->SetStepToComputeProbability(step);
    step = Add(new LinearStep<Ascon::Sigma<0, RotVal[2], RotVal[3]>>(
        word_size_, B[i - 1][1], A[i][1]));
    A[i][1]->SetStepToComputeProbability(step);
    step = Add(new LinearStep<Ascon::Sigma<0, RotVal[4], RotVal[5]>>(
        word_size_, B[i - 1][2], A[i][2]));
    A[i][2]->SetStepToComputeProbability(step);
    step = Add(new LinearStep<Ascon::Sigma<0, RotVal[6], RotVal[7]>>(
        word_size_, B[i - 1][3], A[i][3]));
    A[i][3]->SetStepToComputeProbability(step);
    step = Add(new LinearStep<Ascon::Sigma<0, RotVal[8], RotVal[9]>>(
        word_size_, B[i - 1][4], A[i][4]));
    A[i][4]->SetStepToComputeProbability(step);

    Add(new BitsliceStep<Ascon::isActive>(word_size_, A[i][0], A[i][1], A[i][2],
                                          A[i][3], A[i][4], S[i]));
  }
}

bool Ascon::Callback(std::string function, Characteristic& characteristic,
                     std::mt19937& rng, Logfile& logfile) {
  if (function.empty())
    return true;
  else if (function.compare("found") == 0) {
    uint64_t cost = 0;
    uint64_t trunc_diff[20];
    uint64_t trunc_cost[20];
    for (int r = 0; r <= num_rounds_; r++) {
      trunc_diff[r] = characteristic.GetConditionWordMask(
          [](BitCondition bc) { return bc == BitCondition("1"); },
          characteristic.GetCrypto()->GetConditionWordIndex("S", r));
      trunc_cost[r] = nldtool::HammingWeight(trunc_diff[r]);
      cost += trunc_cost[r];
    }
    if (cost < best_char_) {
      logfile << "found new characteristic" << std::endl;
      logfile << "cost: " << cost << std::endl;
      characteristic.WriteCharacteristic(logfile);
      best_char_ = cost;
    }
    return true;
  }
  assert(false);
  return false;
}

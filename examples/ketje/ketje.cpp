#include "ketje/ketje.h"

#include "bitslice_step.h"
#include "functions.h"
#include "linear_step.h"
#include "logfile.h"

constexpr uint64_t Ketje::RC[24];
constexpr int Ketje::R[5][5];
constexpr int Ketje::pstar[25];

void Ketje::AddToOptions(cxxopts::Options& options) {}

Ketje::Ketje(cxxopts::Options& options)
    : Crypto(options["word-size"].as<int>()),
      num_rounds_(options["num-rounds"].as<int>()),
      rate_(options["rate"].as<int>()),
      best_char_(std::numeric_limits<uint64_t>().max()) {
  InitWords();

  for (int r = 0; r < num_rounds_; r++) {
    // linear layer (theta, rho, pi)
    Add(new LinearStep<LinearThetaRhoPi>(
        word_size_,
        {
            A[r][0][0], A[r][1][0], A[r][2][0], A[r][3][0], A[r][4][0],
            A[r][0][1], A[r][1][1], A[r][2][1], A[r][3][1], A[r][4][1],
            A[r][0][2], A[r][1][2], A[r][2][2], A[r][3][2], A[r][4][2],
            A[r][0][3], A[r][1][3], A[r][2][3], A[r][3][3], A[r][4][3],
            A[r][0][4], A[r][1][4], A[r][2][4], A[r][3][4], A[r][4][4],
            B[r][0][0], B[r][1][0], B[r][2][0], B[r][3][0], B[r][4][0],
            B[r][0][1], B[r][1][1], B[r][2][1], B[r][3][1], B[r][4][1],
            B[r][0][2], B[r][1][2], B[r][2][2], B[r][3][2], B[r][4][2],
            B[r][0][3], B[r][1][3], B[r][2][3], B[r][3][3], B[r][4][3],
            B[r][0][4], B[r][1][4], B[r][2][4], B[r][3][4], B[r][4][4],
        }));

    // nonlinear layer (chi,iota)
    for (int y = 0; y < 5; y++) {
      Add(new BitsliceStep<CHIRC>(
          word_size_,
          {B[r][0][y], B[r][1][y], B[r][2][y], B[r][3][y], B[r][4][y],  //
           y == 0 ? K[0] : K0,                                          //
           C[r][0][y], C[r][1][y], C[r][2][y], C[r][3][y], C[r][4][y]}));

      Add(new BitsliceStep<isActive>(
          word_size_, {B[r][0][y], B[r][1][y], B[r][2][y], B[r][3][y],
                       B[r][4][y], S[r][y]}));
    }

    // message mapping
    for (int x = 0; x < 25; x++) {
      if (x < rate_ / word_size_) {
        // leave undefined = message input
      } else {
        // map to next phase
        int index = pstar[x];
        Add(new BitsliceStep<ID>(word_size_, C[r][index % 5][index / 5],
                                 A[r + 1][index % 5][index / 5]));
      }
    }
  }
}

void Ketje::InitWords() {
  int row = 0;
  // constants
  K0 = ConditionWordPtr(new ConditionWord(0, 64));
  for (int r = 0; r < num_rounds_; r++)
    K[r] = ConditionWordPtr(new ConditionWord(RC[r], 64));
  // initial value
  for (int y = 0; y < 5; y++, row++)
    for (int x = 0; x < 5; x++)
      A[0][x][y] = AddConditionWord(GetName('A', y, x), 0, row, x);
  // permutations for each block
  for (int r = 0; r < num_rounds_; r++) {
    for (int y = 0; y < 5; y++)
      for (int x = 0; x < 5; x++)
        B[r][y][(2 * x + 3 * y) % 5] = AddConditionWord(
            GetName('B', y, x), r, row + y, (2 * x + 3 * y) % 5, SUBWORD);
    row += 5;
    for (int y = 0; y < 5; y++)
      for (int x = 0; x < 5; x++)
        C[r][y][(2 * x + 3 * y) % 5] = AddConditionWord(
            GetName('C', y, x), r, row + y, (2 * x + 3 * y) % 5, SUBWORD);
    row += 5;

    for (int x = 0; x < 5; x++)
      S[r][x] = AddConditionWord(GetName('S', 0, x), r, row, x);
    row++;
    for (int y = 0; y < 5; y++, row++)
      for (int x = 0; x < 5; x++)
        A[r + 1][x][y] = AddConditionWord(GetName('A', y, x), r + 1, row, x);
  }
}

std::string Ketje::GetName(char a, int i, int j) {
  std::string name(1, a);
  // name = name + "[" + std::to_string(j) + "][" + std::to_string(i) + "]";
  name = name + std::to_string(i) + std::to_string(j);
  return name;
}

bool Ketje::Callback(std::string function, Characteristic& characteristic,
                     std::mt19937& rng, Logfile& logfile) {
  if (function.empty())
    return true;
  else if (function.compare("found") == 0) {
    uint64_t cost = 0;
    uint64_t trunc_diff[20][5];
    uint64_t trunc_cost[20][5];
    for (int r = 0; r < num_rounds_; r++) {
      for (int x = 0; x < 5; x++) {
        trunc_diff[r][x] = characteristic.GetConditionWordMask(
            [](BitCondition bc) { return bc == BitCondition("1"); },
            characteristic.GetCrypto()->GetConditionWordIndex(
                GetName('S', 0, x), r));
        trunc_cost[r][x] = nldtool::HammingWeight(trunc_diff[r][x]);
        cost += trunc_cost[r][x];
      }
    }
    if (cost < best_char_) {
      logfile << "found new characteristic" << std::endl;
      logfile << "cost: " << cost << std::endl;
      characteristic.WriteCharacteristic(logfile);
      best_char_ = cost;
    }
    return true;
  } else if (function == "setupchar") {
    // logfile<<"setting up chars for rate " << rate_ << " and word_size " <<
    // word_size_ <<std::endl;
    S[0][0]->GetConditionProxy(0)->SetCondition(characteristic,
                                                BitCondition("1"));
    for (int y = 0; y < 5; y++)
      for (int x = 0; x < 5; x++) {
        if (y * 5 + x < rate_ / word_size_) {
          // fill with ?
          int index = pstar[y * 5 + x];
          for (int bit = 0; bit < word_size_; bit++) {
            // set 1 difference bit at beginning
            // if(x==0 && y ==0 && bit == 0)
            //  A[0][index / 5][index %
            //  5]->GetConditionProxy(bit)->SetCondition(characteristic,
            //  BitCondition("x"));
            // else
            A[0][index % 5][index / 5]->GetConditionProxy(bit)->SetCondition(
                characteristic, BitCondition("?"));
          }

        } else {
          // fill with -
          int index = pstar[y * 5 + x];
          for (int bit = 0; bit < word_size_; bit++)
            A[0][index % 5][index / 5]->GetConditionProxy(bit)->SetCondition(
                characteristic, BitCondition("-"));
        }
        // last round should not have any message difference
        for (int bit = 0; bit < word_size_; bit++)
          A[num_rounds_][x][y]->GetConditionProxy(bit)->SetCondition(
              characteristic, BitCondition("-"));
      }
    // characteristic.WriteCharacteristic(logfile);
    // logfile<<"setup done!"<<std::endl;
  }
  assert(false);
  return false;
}

class Ketje::isActive : public F {
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

constexpr char Ketje::isActive::kName[];

class Ketje::ID : public F {
 public:
  static constexpr char kName[] = "ID";
  static const int kNumInputs = 1;
  static const int kNumOutputs = 1;
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    y[0] = x[0];
  }
};

class Ketje::CHIRC : public F {
 public:
  static constexpr char kName[] = "CHIRC";
  static const int kNumInputs = 6;
  static const int kNumOutputs = 5;
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    // x[0..4]: input
    // x[5]:    round constant
    // y[0..4]: output
    for (int i = 0; i < 5; ++i)
      y[i] = (~x[(i + 1) % 5] & x[(i + 2) % 5]) ^ x[i];
    y[0] ^= x[5];  // round constant
  }
};

class Ketje::LinearThetaRhoPi : public F {
 public:
  static constexpr char kName[] = "LinearThetaRhoPi";
  static const int kNumInputs = 25;
  static const int kNumOutputs = 25;
  template <class T>
  static void f(int word_size, const T x[kNumInputs], T y[kNumOutputs]) {
    T t[25], p[5];
    // parity of each lane
    p[0] = x[0] ^ x[5] ^ x[10] ^ x[15] ^ x[20];
    p[1] = x[1] ^ x[6] ^ x[11] ^ x[16] ^ x[21];
    p[2] = x[2] ^ x[7] ^ x[12] ^ x[17] ^ x[22];
    p[3] = x[3] ^ x[8] ^ x[13] ^ x[18] ^ x[23];
    p[4] = x[4] ^ x[9] ^ x[14] ^ x[19] ^ x[24];
    // theta w/o parity
    t[0] = x[0] ^ p[4] ^ nldtool::Rotl(p[1], 1, word_size);
    t[1] = x[1] ^ p[0] ^ nldtool::Rotl(p[2], 1, word_size);
    t[2] = x[2] ^ p[1] ^ nldtool::Rotl(p[3], 1, word_size);
    t[3] = x[3] ^ p[2] ^ nldtool::Rotl(p[4], 1, word_size);
    t[4] = x[4] ^ p[3] ^ nldtool::Rotl(p[0], 1, word_size);
    t[5] = x[5] ^ p[4] ^ nldtool::Rotl(p[1], 1, word_size);
    t[6] = x[6] ^ p[0] ^ nldtool::Rotl(p[2], 1, word_size);
    t[7] = x[7] ^ p[1] ^ nldtool::Rotl(p[3], 1, word_size);
    t[8] = x[8] ^ p[2] ^ nldtool::Rotl(p[4], 1, word_size);
    t[9] = x[9] ^ p[3] ^ nldtool::Rotl(p[0], 1, word_size);
    t[10] = x[10] ^ p[4] ^ nldtool::Rotl(p[1], 1, word_size);
    t[11] = x[11] ^ p[0] ^ nldtool::Rotl(p[2], 1, word_size);
    t[12] = x[12] ^ p[1] ^ nldtool::Rotl(p[3], 1, word_size);
    t[13] = x[13] ^ p[2] ^ nldtool::Rotl(p[4], 1, word_size);
    t[14] = x[14] ^ p[3] ^ nldtool::Rotl(p[0], 1, word_size);
    t[15] = x[15] ^ p[4] ^ nldtool::Rotl(p[1], 1, word_size);
    t[16] = x[16] ^ p[0] ^ nldtool::Rotl(p[2], 1, word_size);
    t[17] = x[17] ^ p[1] ^ nldtool::Rotl(p[3], 1, word_size);
    t[18] = x[18] ^ p[2] ^ nldtool::Rotl(p[4], 1, word_size);
    t[19] = x[19] ^ p[3] ^ nldtool::Rotl(p[0], 1, word_size);
    t[20] = x[20] ^ p[4] ^ nldtool::Rotl(p[1], 1, word_size);
    t[21] = x[21] ^ p[0] ^ nldtool::Rotl(p[2], 1, word_size);
    t[22] = x[22] ^ p[1] ^ nldtool::Rotl(p[3], 1, word_size);
    t[23] = x[23] ^ p[2] ^ nldtool::Rotl(p[4], 1, word_size);
    t[24] = x[24] ^ p[3] ^ nldtool::Rotl(p[0], 1, word_size);
    // rho, pi
    for (int j = 0; j < 5; j++)
      for (int i = 0; i < 5; i++)
        y[5 * (((2 * i + 3 * j) % 5)) + j] =
            nldtool::Rotl(t[i + 5 * j], R[i][j] % word_size, word_size);
  }
};

constexpr char Ketje::ID::kName[];
constexpr char Ketje::CHIRC::kName[];
constexpr char Ketje::LinearThetaRhoPi::kName[];

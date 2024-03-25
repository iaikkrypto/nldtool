#include "sm3/sm3.h"

#include "bitslice_step.h"
#include "carry_step.h"
#include "functions.h"
#include "horizontal_condition_word.h"
#include "linear_step.h"
#include "text_io_format.h"

const uint64_t Sm3::K256[64] = {
    0x79cc4519, 0xf3988a32, 0xe7311465, 0xce6228cb, 0x9cc45197, 0x3988a32f,
    0x7311465e, 0xe6228cbc, 0xcc451979, 0x988a32f3, 0x311465e7, 0x6228cbce,
    0xc451979c, 0x88a32f39, 0x11465e73, 0x228cbce6, 0x9d8a7a87, 0x3b14f50f,
    0x7629ea1e, 0xec53d43c, 0xd8a7a879, 0xb14f50f3, 0x629ea1e7, 0xc53d43ce,
    0x8a7a879d, 0x14f50f3b, 0x29ea1e76, 0x53d43cec, 0xa7a879d8, 0x4f50f3b1,
    0x9ea1e762, 0x3d43cec5, 0x7a879d8a, 0xf50f3b14, 0xea1e7629, 0xd43cec53,
    0xa879d8a7, 0x50f3b14f, 0xa1e7629e, 0x43cec53d, 0x879d8a7a, 0x0f3b14f5,
    0x1e7629ea, 0x3cec53d4, 0x79d8a7a8, 0xf3b14f50, 0xe7629ea1, 0xcec53d43,
    0x9d8a7a87, 0x3b14f50f, 0x7629ea1e, 0xec53d43c, 0xd8a7a879, 0xb14f50f3,
    0x629ea1e7, 0xc53d43ce, 0x8a7a879d, 0x14f50f3b, 0x29ea1e76, 0x53d43cec,
    0xa7a879d8, 0x4f50f3b1, 0x9ea1e762, 0x3d43cec5};

const int Sm3::I = 4;
const int Sm3::L = 7;

class Sm3::P0 : public F {
 public:
  static constexpr char kName[] = "P0";
  static const int kNumInputs = 1;
  static const int kNumOutputs = 1;
  template <class T>
  static void f(int word_size, const T x[kNumInputs], T y[kNumOutputs]) {
    y[0] = x[0] ^ nldtool::Rotl<32>(x[0], 9) ^ nldtool::Rotl<32>(x[0], 17);
  }
};

constexpr char Sm3::P0::kName[];

class Sm3::P1 : public F {
 public:
  static constexpr char kName[] = "P1";
  static const int kNumInputs = 1;
  static const int kNumOutputs = 1;
  template <class T>
  static void f(int word_size, const T x[kNumInputs], T y[kNumOutputs]) {
    y[0] = x[0] ^ nldtool::Rotl<32>(x[0], 15) ^ nldtool::Rotl<32>(x[0], 23);
  }
};

constexpr char Sm3::P1::kName[];

class Sm3::ADD3XOR2 : public F {
 public:
  static constexpr char kName[] = "ADD3XOR2";
  static const int kNumInputs = 5;
  static const int kNumOutputs = 3;
  static constexpr int Symmetry(int i) { return (i < 3) ? 0 : i; }
  static constexpr int Bitsize(int i) { return (i == 4 || i == 7) ? 2 : 1; }
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    const int cib = kNumInputs - 1;
    const int sb = 0;
    const int sa = 1;
    const int cob = 2;
    y[sb] = x[0] + x[1] + x[2] + x[cib];
    y[cob] = y[sb] >> 1;
    y[sb] &= 1;
    y[sa] = (y[sb] ^ x[3]);
    y[sa] &= 1;
  }
};

constexpr char Sm3::ADD3XOR2::kName[];

class Sm3::ADD3XOR2SP : public F {
 public:
  static constexpr char kName[] = "ADD3XOR2SP";
  static const int kNumInputs = 5;
  static const int kNumOutputs = 4;
  static constexpr int Symmetry(int i) { return (i < 2) ? 0 : i; }
  static constexpr int Bitsize(int i) { return (i == 4 || i == 8) ? 2 : 1; }
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    const int cib = kNumInputs - 1;
    const int sc = 0;
    const int sb = 1;
    const int sa = 2;
    const int cob = 3;
    y[sc] = x[0] + x[1] + (x[cib] & 1);
    y[cob] = y[sc] >> 1;
    y[sc] &= 1;
    y[sb] = y[sc] + x[2] + (x[cib] >> 1);
    y[cob] |= y[sb] & 2;
    y[sb] &= 1;
    y[sa] = (y[sb] ^ x[3]);
    y[sa] &= 1;
  }
};

constexpr char Sm3::ADD3XOR2SP::kName[];

class Sm3::ADD2XOR2ADD2SP : public F {
 public:
  static constexpr char kName[] = "ADD2XOR2ADD2SP";
  static const int kNumInputs = 5;
  static const int kNumOutputs = 4;
  static constexpr int Symmetry(int i) { return (i < 2) ? 0 : i; }
  static constexpr int Bitsize(int i) { return (i == 4 || i == 8) ? 2 : 1; }
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    const int cib = kNumInputs - 1;
    const int sc = 0;
    const int sb = 1;
    const int sa = 2;
    const int cob = 3;
    y[sc] = x[0] + x[1] + (x[cib] & 1);
    y[cob] = y[sc] >> 1;
    y[sc] &= 1;
    y[sb] = (y[sc] ^ x[2]);
    y[sb] &= 1;
    y[sa] = y[sb] + x[3] + (x[cib] >> 1);
    y[cob] |= y[sa] & 2;
    y[sa] &= 1;
  }
};

constexpr char Sm3::ADD2XOR2ADD2SP::kName[];

class Sm3::XOR2ADD3SP : public F {
 public:
  static constexpr char kName[] = "XOR2ADD3SP";
  static const int kNumInputs = 5;
  static const int kNumOutputs = 4;
  static constexpr int Symmetry(int i) { return (i < 2) ? 0 : i; }
  static constexpr int Bitsize(int i) { return (i == 4 || i == 8) ? 2 : 1; }
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    const int cib = kNumInputs - 1;
    const int sc = 0;
    const int sb = 1;
    const int sa = 2;
    const int cob = 3;
    y[sc] = (x[0] ^ x[1]);
    y[sc] &= 1;
    y[sb] = x[2] + y[sc] + (x[cib] & 1);
    y[cob] = y[sb] >> 1;
    y[sb] &= 1;
    y[sa] = y[sb] + x[3] + (x[cib] >> 1);
    y[cob] |= y[sa] & 2;
    y[sa] &= 1;
  }
};

constexpr char Sm3::XOR2ADD3SP::kName[];

class Sm3::ADD4SP : public F {
 public:
  static constexpr char kName[] = "ADD4SP";
  static const int kNumInputs = 5;
  static const int kNumOutputs = 4;
  static constexpr int Symmetry(int i) { return (i < 2) ? 0 : i; }
  static constexpr int Bitsize(int i) { return (i == 4 || i == 8) ? 3 : 1; }
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    const int cib = kNumInputs - 1;
    const int sc = 0;
    const int sb = 1;
    const int sa = 2;
    const int cob = 3;
    y[sc] = x[0] + x[1] + (x[cib] & 1);
    y[cob] = y[sc] >> 1;
    y[sc] &= 1;
    y[sb] = y[sc] + x[2] + ((x[cib] >> 1) & 1);
    y[cob] |= y[sb] & 2;
    y[sb] &= 1;
    y[sa] = y[sb] + x[3] + ((x[cib] >> 2) & 1);
    y[cob] |= (y[sa] << 1) & 4;
    y[sa] &= 1;
  }
};

constexpr char Sm3::ADD4SP::kName[];

class Sm3::IFB : public F {
 public:
  static constexpr char kName[] = "IFB";
  static const int kNumInputs = 3;
  static const int kNumOutputs = 1;
  static constexpr int Bitsize(int i) { return 2; }
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    const T& a = x[0];
    const T& b = x[1];
    const T& c = x[2];
    T& r = y[0];
    r = (a & b) ^ (a & c) ^ c;
  }
};

constexpr char Sm3::IFB::kName[];

class Sm3::MAJB : public F {
 public:
  static constexpr char kName[] = "MAJB";
  static const int kNumInputs = 3;
  static const int kNumOutputs = 1;
  static constexpr int Bitsize(int i) { return 2; }
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    const T& a = x[0];
    const T& b = x[1];
    const T& c = x[2];
    T& r = y[0];
    r = (a & b) ^ (b & c) ^ (c & a);
  }
};

constexpr char Sm3::MAJB::kName[];

void Sm3::AddToOptions(cxxopts::Options& options) {
  options.add_options("SM3 specific")                //
      ("sm3-bs",                                     //
       "bitslice options",                           //
       cxxopts::value<int>()->default_value("260"),  //
       "N");
}

Sm3::Sm3(cxxopts::Options& options)
    : Crypto(options["word-size"].as<int>()),
      num_rounds_(options["num-rounds"].as<int>()),
      E_(tE_ + I),
      A_(tA_ + I),
      E2_(tE2_ + I),
      A2_(tA2_ + I) {
  int bitslice_flags = options["sm3-bs"].as<int>();

  int twobit_flag = (bitslice_flags & 0x300) != 0;

  if (twobit_flag) {
    // instantiate words
    for (int i = 0; i < 16; i++) {
      W2_[i] = AddConditionWord("W2", i, -1, -1, HORIZONTALWORD);
      W_[i] = AddReferenceToConditionWord(W2_[i], "W", i, I + L * i + 2, 2,
                                          MAINWORD);
    }
    for (int i = -I; i < 0; i++) {
      E2_[i] = AddConditionWord("E2", i, -1, -1, HORIZONTALWORD);
      E_[i] = AddReferenceToConditionWord(E2_[i], "E", i, I + i, 1, MAINWORD);
    }
    for (int i = -I; i < 0; i++) {
      A2_[i] = AddConditionWord("A2", i, -1, -1, HORIZONTALWORD);
      A_[i] = AddReferenceToConditionWord(A2_[i], "A", i, I + i, 0, MAINWORD);
    }

    for (int i = 16; i < num_rounds_ + 4; i++) {
      if (i < num_rounds_) {
        W2_[i] = AddConditionWord("W2", i, -1, -1, HORIZONTALWORD);
        W_[i] = AddReferenceToConditionWord(W2_[i], "W", i, I + L * i + 2, 2,
                                            MAINWORD);
        S02_[i] = AddConditionWord("S02", i, -1, -1, HORIZONTALWORD);
        S0_[i] = AddReferenceToConditionWord(S02_[i], "S0", i, I + L * i + 0, 2,
                                             SUBWORD);
        P12_[i] = AddConditionWord("P12", i, -1, -1, HORIZONTALWORD);
        P1_[i] = AddReferenceToConditionWord(P12_[i], "P1", i, I + L * i + 1, 2,
                                             SUBWORD);
      } else {
        W2_[i] = AddConditionWord("W2", i, -1, -1, HORIZONTALWORD);
        W_[i] = AddReferenceToConditionWord(W2_[i], "W", i, -1, -1, SUBWORD);
        S02_[i] = AddConditionWord("S02", i, -1, -1, HORIZONTALWORD);
        S0_[i] = AddReferenceToConditionWord(S02_[i], "S0", i, -1, -1, SUBWORD);
        P12_[i] = AddConditionWord("P12", i, -1, -1, HORIZONTALWORD);
        P1_[i] = AddReferenceToConditionWord(P12_[i], "P1", i, -1, -1, SUBWORD);
      }
    }

    for (int i = 0; i < num_rounds_; i++) {
      Wp2_[i] = AddConditionWord("Wp2", i, -1, -1, HORIZONTALWORD);
      Wp_[i] = AddReferenceToConditionWord(Wp2_[i], "Wp", i, I + L * i + 2, 3,
                                           SUBWORD);
      T2_[i] = AddConditionWord("T2", i, -1, -1, HORIZONTALWORD);
      T_[i] = AddReferenceToConditionWord(T2_[i], "T", i, I + L * i + 1, 0,
                                          SUBWORD);
      F2_[i] = AddConditionWord("F2", i, -1, -1, HORIZONTALWORD);
      F_[i] = AddReferenceToConditionWord(F2_[i], "F", i, I + L * i + 0, 0,
                                          SUBWORD);
      A2_[i] = AddConditionWord("A2", i, -1, -1, HORIZONTALWORD);
      A_[i] = AddReferenceToConditionWord(A2_[i], "A", i, I + L * i + 2, 0,
                                          MAINWORD);
      G2_[i] = AddConditionWord("G2", i, -1, -1, HORIZONTALWORD);
      G_[i] = AddReferenceToConditionWord(G2_[i], "G", i, I + L * i + 0, 1,
                                          SUBWORD);
      P2_[i] = AddConditionWord("P2", i, -1, -1, HORIZONTALWORD);
      P_[i] = AddReferenceToConditionWord(P2_[i], "P", i, I + L * i + 1, 1,
                                          SUBWORD);
      E2_[i] = AddConditionWord("E2", i, -1, -1, HORIZONTALWORD);
      E_[i] = AddReferenceToConditionWord(E2_[i], "E", i, I + L * i + 2, 1,
                                          MAINWORD);
      TK2_[i] = AddConditionWord("TK2", i, -1, -1, HORIZONTALWORD);
      TK_[i] = AddReferenceToConditionWord(TK2_[i], "TK", i, I + L * i + 1, 3,
                                           SUBWORD, 1);
      TA2_[i] = AddConditionWord("TA2", i, -1, -1, HORIZONTALWORD);
      TA_[i] =
          AddReferenceToConditionWord(TA2_[i], "TA", i, -1, -1, CARRYWORD, 1);
      FD2_[i] = AddConditionWord("FD2", i, -1, -1, HORIZONTALWORD);
      FD_[i] =
          AddReferenceToConditionWord(FD2_[i], "FD", i, -1, -1, CARRYWORD, 1);
      FDTA2_[i] = AddConditionWord("FDTA2", i, -1, -1, HORIZONTALWORD);
      FDTA_[i] = AddReferenceToConditionWord(FDTA2_[i], "FDTA", i, -1, -1,
                                             CARRYWORD, 1);
      GH2_[i] = AddConditionWord("GH2", i, -1, -1, HORIZONTALWORD);
      GH_[i] =
          AddReferenceToConditionWord(GH2_[i], "GH", i, -1, -1, CARRYWORD, 1);
      GHT2_[i] = AddConditionWord("GHT2", i, -1, -1, HORIZONTALWORD);
      GHT_[i] =
          AddReferenceToConditionWord(GHT2_[i], "GHT", i, -1, -1, CARRYWORD, 1);
    }
  } else {
    // instantiate words
    for (int i = 0; i < 16; i++)
      W_[i] = AddConditionWord("W", i, I + L * i + 2, 2);
    for (int i = -I; i < 0; i++) E_[i] = AddConditionWord("E", i, I + i, 1);
    for (int i = -I; i < 0; i++) A_[i] = AddConditionWord("A", i, I + i, 0);

    for (int i = 16; i < num_rounds_ + 4; i++) {
      if (i < num_rounds_) {
        W_[i] = AddConditionWord("W", i, I + L * i + 2, 2, MAINWORD);
        S0_[i] = AddConditionWord("S0", i, I + L * i + 0, 2, SUBWORD);
        P1_[i] = AddConditionWord("P1", i, I + L * i + 1, 2, SUBWORD);
      } else {
        W_[i] = AddConditionWord("W", i, -1, -1, SUBWORD);
        S0_[i] = AddConditionWord("S0", i, -1, -1, SUBWORD);
        P1_[i] = AddConditionWord("P1", i, -1, -1, SUBWORD);
      }
    }

    for (int i = 0; i < num_rounds_; i++) {
      Wp_[i] = AddConditionWord("Wp", i, I + L * i + 2, 3, SUBWORD);
      T_[i] = AddConditionWord("T", i, I + L * i + 1, 0, SUBWORD);
      F_[i] = AddConditionWord("F", i, I + L * i + 0, 0, SUBWORD);
      A_[i] = AddConditionWord("A", i, I + L * i + 2, 0);
      G_[i] = AddConditionWord("G", i, I + L * i + 0, 1, SUBWORD);
      P_[i] = AddConditionWord("P", i, I + L * i + 1, 1, SUBWORD);
      E_[i] = AddConditionWord("E", i, I + L * i + 2, 1);
      TK_[i] = AddConditionWord("TK", i, I + L * i + 1, 3, SUBWORD, 1);
      TA_[i] = AddConditionWord("TA", i, -1, -1, CARRYWORD, 1);
      FD_[i] = AddConditionWord("FD", i, -1, -1, CARRYWORD, 1);
      FDTA_[i] = AddConditionWord("FDTA", i, -1, -1, CARRYWORD, 1);
      GH_[i] = AddConditionWord("GH", i, -1, -1, CARRYWORD, 1);
      GHT_[i] = AddConditionWord("GHT", i, -1, -1, CARRYWORD, 1);
    }
  }
  for (int i = 0; i < num_rounds_; i++) {
    CAo_[i] = AddConditionWord("CAo", i, I + L * i + 3, 0, CARRYWORD, 3);
    CPo_[i] = AddConditionWord("CPo", i, I + L * i + 3, 1, CARRYWORD, 3);
    CAi_[i] = CAo_[i]->Rotl(1)->SetZero(7, 1, 0)->SetZero(0, 2, 1);
    CPi_[i] = CPo_[i]->Rotl(1)->SetZero(7, 1, 0)->SetZero(0, 2, 1);
    CA_[i] = AddReferenceToConditionWord(CAo_[i], "CX", i, I + L * i + 5, 0,
                                         CARRYWORD, 2, 1);
    CP_[i] = AddReferenceToConditionWord(CPo_[i], "CY", i, I + L * i + 5, 1,
                                         CARRYWORD, 2, 1);
    CT_[i] = AddConditionWord("CZ", i, I + L * i + 4, 0, CARRYWORD, 2);
    CTA_[i] = AddConditionWord("CTA", i, I + L * i + 5, 2, CARRYWORD, 2);
    CTP_[i] = AddConditionWord("CTP", i, I + L * i + 5, 3, CARRYWORD, 2);
    CTA6_[i] = AddConditionWord("CTA6", i, I + L * i + 6, 2, CARRYWORD, 3);
    CTP6_[i] = AddConditionWord("CTP6", i, I + L * i + 6, 3, CARRYWORD, 3);
  }
  // message expansion
  for (int i = 16; i < num_rounds_ + 4; i++) {
    Add(new BitsliceStep<XOR<3>>(word_size_, W_[i - 16], W_[i - 9],
                                 W_[i - 3]->Rotr(32 - 15), S0_[i]));
    Add(new BitsliceStep<XOR<3>>(word_size_, S0_[i], S0_[i]->Rotr(32 - 15),
                                 S0_[i]->Rotr(32 - 23), P1_[i]));
    Add(new LinearStep<P1>(word_size_, S0_[i], P1_[i]));
    Add(new BitsliceStep<XOR<3>>(word_size_, P1_[i], W_[i - 13]->Rotr(32 - 7),
                                 W_[i - 6], W_[i]));
  }
  if (twobit_flag) {
    for (int i = 16; i < num_rounds_ + 4; i++) {
      Add(new BitsliceStep<XORB<3>>(word_size_ - 1, W2_[i - 16], W2_[i - 9],
                                    W2_[i - 3]->Rotl(15), S02_[i]));
      Add(new BitsliceStep<XORB<3>>(word_size_ - 1, S02_[i], S02_[i]->Rotl(15),
                                    S02_[i]->Rotl(23), P12_[i]));
      Add(new BitsliceStep<XORB<3>>(word_size_ - 1, P12_[i],
                                    W2_[i - 13]->Rotl(7), W2_[i - 6], W2_[i]));
    }
  }

  // characteristic update
  for (int i = 0; i < num_rounds_; i++) {
    // Always linear round for sigma
    Add(new LinearStep<P0>(word_size_, P_[i], E_[i]));

    if (bitslice_flags_ & 0x4) trivialBitSlice(i);
    if (bitslice_flags_ & 0x1000) oneRound4In(i);
    if (bitslice_flags_ & 0x8) oneRound4InSp(i);
    if (bitslice_flags_ & 0x100) trivialTwoBitSlice(i);
    if (bitslice_flags_ & 0x200) oneRound4InTBS(i);
  }

  //  // feed-forward
  for (int i = 0; i < 4; i++) {
    A_[num_rounds_ + i] = AddConditionWord("HA", i, num_rounds_ * L + I + i, 0);
    E_[num_rounds_ + i] = AddConditionWord("HE", i, num_rounds_ * L + I + i, 1);
  }
  Add(new BitsliceStep<XOR<2>>(word_size_, E_[-4]->Rotl(19),
                               E_[num_rounds_ - 4]->Rotl(19),
                               E_[num_rounds_ + I - 4]));
  Add(new BitsliceStep<XOR<2>>(word_size_, E_[-3]->Rotl(19),
                               E_[num_rounds_ - 3]->Rotl(19),
                               E_[num_rounds_ + I - 3]));
  Add(new BitsliceStep<XOR<2>>(word_size_, E_[-2], E_[num_rounds_ - 2],
                               E_[num_rounds_ + I - 2]));
  Add(new BitsliceStep<XOR<2>>(word_size_, E_[-1], E_[num_rounds_ - 1],
                               E_[num_rounds_ + I - 1]));
  Add(new BitsliceStep<XOR<2>>(word_size_, A_[-4]->Rotl(9),
                               A_[num_rounds_ - 4]->Rotl(9),
                               A_[num_rounds_ + I - 4]));
  Add(new BitsliceStep<XOR<2>>(word_size_, A_[-3]->Rotl(9),
                               A_[num_rounds_ - 3]->Rotl(9),
                               A_[num_rounds_ + I - 3]));
  Add(new BitsliceStep<XOR<2>>(word_size_, A_[-2], A_[num_rounds_ - 2],
                               A_[num_rounds_ + I - 2]));
  Add(new BitsliceStep<XOR<2>>(word_size_, A_[-1], A_[num_rounds_ - 1],
                               A_[num_rounds_ + I - 1]));
}

void Sm3::oneRound4InTBS(int round) {
  ConditionWordPtr H2 = E2_[round - 4]->Rotl(19);
  ConditionWordPtr G2 = E2_[round - 3]->Rotl(19);
  ConditionWordPtr F2 = E2_[round - 2];
  ConditionWordPtr E2 = E2_[round - 1];
  ConditionWordPtr D2 = A2_[round - 4]->Rotl(9);
  ConditionWordPtr C2 = A2_[round - 3]->Rotl(9);
  ConditionWordPtr B2 = A2_[round - 2];
  ConditionWordPtr A2 = A2_[round - 1];

  ConditionWordPtr K2 =
      ConditionWordPtr(new HorizontalConditionWord(K256[round], word_size_));

  Add(new BitsliceStep<ADDB<3>>(word_size_ - 1, A2->Rotl(12), E2, K2,
                                CT_[round]->Shl(1), T2_[round], CT_[round],
                                CT_[round]->Shr(1)));

  Add(new BitsliceStep<ADDB<4>>(word_size_ - 1, H2, T2_[round]->Rotl(7),
                                G2_[round], W2_[round], CP_[round]->Shl(1),
                                P2_[round], CP_[round], CP_[round]->Shr(1)));

  Add(new BitsliceStep<ADDB<4>>(word_size_ - 1, D2, TA2_[round], F2_[round],
                                Wp2_[round], CA_[round]->Shl(1), A2_[round],
                                CA_[round], CA_[round]->Shr(1)));

  if ((bitslice_flags_ & 0x100) == 0) {
    Add(new BitsliceStep<XORB<2>>(word_size_ - 1, W2_[round], W2_[round + 4],
                                  Wp2_[round]));
    if (round < 16) {
      Add(new BitsliceStep<XORB<3>>(word_size_ - 1, A2, B2, C2, F2_[round]));
      Add(new BitsliceStep<XORB<3>>(word_size_ - 1, E2, F2, G2, G2_[round]));

    } else {
      Add(new BitsliceStep<MAJB>(word_size_ - 1, A2, B2, C2, F2_[round]));

      Add(new BitsliceStep<IFB>(word_size_ - 1, E2, F2, G2, G2_[round]));
    }

    Add(new BitsliceStep<XORB<2>>(word_size_ - 1, A2->Rotl(12),
                                  T2_[round]->Rotl(7), TA2_[round]));

    Add(new BitsliceStep<XORB<3>>(word_size_ - 1, P2_[round],
                                  P2_[round]->Rotl(9), P2_[round]->Rotl(17),
                                  E2_[round]));
  }
}

void Sm3::trivialTwoBitSlice(int round) {
  ConditionWordPtr H2 = E2_[round - 4]->Rotl(19);
  ConditionWordPtr G2 = E2_[round - 3]->Rotl(19);
  ConditionWordPtr F2 = E2_[round - 2];
  ConditionWordPtr E2 = E2_[round - 1];
  ConditionWordPtr D2 = A2_[round - 4]->Rotl(9);
  ConditionWordPtr C2 = A2_[round - 3]->Rotl(9);
  ConditionWordPtr B2 = A2_[round - 2];
  ConditionWordPtr A2 = A2_[round - 1];

  ConditionWordPtr K2 =
      ConditionWordPtr(new HorizontalConditionWord(K256[round], word_size_));

  Add(new BitsliceStep<XORB<2>>(word_size_ - 1, W2_[round], W2_[round + 4],
                                Wp2_[round]));
  if (round < 16) {
    Add(new BitsliceStep<XORB<3>>(word_size_ - 1, A2, B2, C2, F2_[round]));
    Add(new BitsliceStep<XORB<3>>(word_size_ - 1, E2, F2, G2, G2_[round]));

  } else {
    Add(new BitsliceStep<MAJB>(word_size_ - 1, A2, B2, C2, F2_[round]));

    Add(new BitsliceStep<IFB>(word_size_ - 1, E2, F2, G2, G2_[round]));
  }

  ConditionWordPtr C = AddConditionWord("CTK2", round, -1, -1, CARRYWORD);
  Add(new BitsliceStep<ADDB<2>>(word_size_ - 1, A2->Rotl(12), K2, C->Shl(1),
                                TK2_[round], C, C->Shr(1)));
  C = AddConditionWord("CT2", round, -1, -1, CARRYWORD);
  Add(new BitsliceStep<ADDB<2>>(word_size_ - 1, E2, TK2_[round], C->Shl(1),
                                T2_[round], C, C->Shr(1)));
  Add(new BitsliceStep<XORB<2>>(word_size_ - 1, A2->Rotl(12),
                                T2_[round]->Rotl(7), TA2_[round]));
  C = AddConditionWord("CFD2", round, -1, -1, CARRYWORD);
  Add(new BitsliceStep<ADDB<2>>(word_size_ - 1, D2, F2_[round], C->Shl(1),
                                FD2_[round], C, C->Shr(1)));
  C = AddConditionWord("CFDTA2", round, -1, -1, CARRYWORD);
  Add(new BitsliceStep<ADDB<2>>(word_size_ - 1, TA2_[round], FD2_[round],
                                C->Shl(1), FDTA2_[round], C, C->Shr(1)));
  C = AddConditionWord("CA2", round, -1, -1, CARRYWORD);
  Add(new BitsliceStep<ADDB<2>>(word_size_ - 1, FDTA2_[round], Wp2_[round],
                                C->Shl(1), A2_[round], C, C->Shr(1)));

  C = AddConditionWord("CGH2", round, -1, -1, CARRYWORD);
  Add(new BitsliceStep<ADDB<2>>(word_size_ - 1, G2_[round], H2, C->Shl(1),
                                GH2_[round], C, C->Shr(1)));
  C = AddConditionWord("CGHT2", round, -1, -1, CARRYWORD);
  Add(new BitsliceStep<ADDB<2>>(word_size_ - 1, GH2_[round],
                                T2_[round]->Rotl(7), C->Shl(1), GHT2_[round], C,
                                C->Shr(1)));
  C = AddConditionWord("CP2", round, -1, -1, CARRYWORD);
  Add(new BitsliceStep<ADDB<2>>(word_size_ - 1, GHT2_[round], W2_[round],
                                C->Shl(1), P2_[round], C, C->Shr(1)));

  Add(new BitsliceStep<XORB<3>>(word_size_ - 1, P2_[round], P2_[round]->Rotl(9),
                                P2_[round]->Rotl(17), E2_[round]));
}

void Sm3::oneRound4InSp(int round) {
  ConditionWordPtr H = E_[round - 4]->Rotl(19);
  ConditionWordPtr G = E_[round - 3]->Rotl(19);
  ConditionWordPtr F = E_[round - 2];
  ConditionWordPtr E = E_[round - 1];
  ConditionWordPtr D = A_[round - 4]->Rotl(9);
  ConditionWordPtr C = A_[round - 3]->Rotl(9);
  ConditionWordPtr B = A_[round - 2];
  ConditionWordPtr A = A_[round - 1];

  ConditionWordPtr CP = AddConditionWord("CPP", round, -1, -1, CARRYWORD, 3);
  Add(new BitsliceStep<ADD4SP>(word_size_, G_[round], H, T_[round]->Rotl(7),
                               W_[round], CP->Shl(1), GH_[round], GHT_[round],
                               P_[round], CP));
  ConditionWordPtr CA = AddConditionWord("CAA", round, -1, -1, CARRYWORD, 3);
  Add(new BitsliceStep<ADD4SP>(word_size_, D, F_[round], TA_[round], Wp_[round],
                               CA->Shl(1), FD_[round], FDTA_[round], A_[round],
                               CA));

  ConditionWordPtr K = ConditionWordPtr(new ConditionWord(K256[round]));
  ConditionWordPtr CT = AddConditionWord("CTT", round, -1, -1, CARRYWORD, 2);
  Add(new BitsliceStep<ADD3XOR2SP>(word_size_, A->Rotl(12)->Rotl(7), K->Rotl(7),
                                   E->Rotl(7), A->Rotl(12), CT->Shl(1)->Rotl(7),
                                   TK_[round]->Rotl(7), T_[round]->Rotl(7),
                                   TA_[round], CT->Rotl(7)));

  ConditionWordPtr CGHTo =
      AddConditionWord("CGHTT", round, -1, -1, CARRYWORD, 3);
  ConditionWordPtr CGHTi = CGHTo->Rotl(1)->SetZero(7, 2, 0)->SetZero(0, 1, 2);
  Add(new BitsliceStep<ADD4SP>(
      word_size_, A->Rotl(12)->Rotl(7), K->Rotl(7), E->Rotl(7), GH_[round],
      CGHTi, TK_[round]->Rotl(7), T_[round]->Rotl(7), GHT_[round], CGHTo));

  ConditionWordPtr CPo = AddConditionWord("CPPo", round, -1, -1, CARRYWORD, 3);
  ConditionWordPtr CPi = CPo->Rotl(1)->SetZero(7, 1, 0)->SetZero(0, 2, 1);
  Add(new BitsliceStep<ADD4SP>(word_size_, TK_[round]->Rotl(7), E->Rotl(7),
                               GH_[round], W_[round], CPi, T_[round]->Rotl(7),
                               GHT_[round], P_[round], CPo));

  ConditionWordPtr CFDTAo =
      AddConditionWord("CFDTAo", round, -1, -1, CARRYWORD, 2);
  ConditionWordPtr CFDTAi = CFDTAo->Rotl(1)->SetZero(7, 1, 0)->SetZero(0, 1, 1);
  Add(new BitsliceStep<ADD2XOR2ADD2SP>(
      word_size_, TK_[round]->Rotl(7), E->Rotl(7), A->Rotl(12), FD_[round],
      CFDTAi, T_[round]->Rotl(7), TA_[round], FDTA_[round], CFDTAo));

  ConditionWordPtr CAo = AddConditionWord("CAAo", round, -1, -1, CARRYWORD, 2);
  ConditionWordPtr CAi = CAo->Shl(1);
  Add(new BitsliceStep<XOR2ADD3SP>(word_size_, T_[round]->Rotl(7), A->Rotl(12),
                                   FD_[round], Wp_[round], CAi, TA_[round],
                                   FDTA_[round], A_[round], CAo));
}

void Sm3::oneRound4In(int round) {
  ConditionWordPtr H = E_[round - 4]->Rotl(19);
  ConditionWordPtr G = E_[round - 3]->Rotl(19);
  ConditionWordPtr F = E_[round - 2];
  ConditionWordPtr E = E_[round - 1];
  ConditionWordPtr D = A_[round - 4]->Rotl(9);
  ConditionWordPtr C = A_[round - 3]->Rotl(9);
  ConditionWordPtr B = A_[round - 2];
  ConditionWordPtr A = A_[round - 1];

  Add(new CarryStep<ADD<4>>(word_size_, H, T_[round]->Rotl(7), G_[round],
                            W_[round], P_[round]));
  Add(new CarryStep<ADD<4>>(word_size_, D, TA_[round], F_[round], Wp_[round],
                            A_[round]));

  ConditionWordPtr K = ConditionWordPtr(new ConditionWord(K256[round]));
  ConditionWordPtr CT = AddConditionWord("CTTT", round, -1, -1, CARRYWORD, 2);
  Add(new BitsliceStep<ADD3XOR2>(word_size_, A->Rotl(12)->Rotl(7), K->Rotl(7),
                                 E->Rotl(7), A->Rotl(12), CT->Shl(1)->Rotl(7),
                                 T_[round]->Rotl(7), TA_[round], CT->Rotl(7)));
}

void Sm3::trivialBitSlice(int round) {
  ConditionWordPtr H = E_[round - 4]->Rotl(19);
  ConditionWordPtr G = E_[round - 3]->Rotl(19);
  ConditionWordPtr F = E_[round - 2];
  ConditionWordPtr E = E_[round - 1];
  ConditionWordPtr D = A_[round - 4]->Rotl(9);
  ConditionWordPtr C = A_[round - 3]->Rotl(9);
  ConditionWordPtr B = A_[round - 2];
  ConditionWordPtr A = A_[round - 1];

  ConditionWordPtr K = ConditionWordPtr(new ConditionWord(K256[round]));

  Add(new BitsliceStep<XOR<2>>(word_size_, W_[round], W_[round + 4],
                               Wp_[round]));
  if (round < 16) {
    Add(new BitsliceStep<XOR<3>>(word_size_, A, B, C, F_[round]));
    Add(new BitsliceStep<XOR<3>>(word_size_, E, F, G, G_[round]));

  } else {
    Add(new BitsliceStep<MAJ>(word_size_, A, B, C, F_[round]));

    Add(new BitsliceStep<IF>(word_size_, E, F, G, G_[round]));
  }

  Add(new CarryStep<ADD<2>>(word_size_, A->Rotl(12), K, TK_[round]));
  Add(new CarryStep<ADD<2>>(word_size_, E, TK_[round], T_[round]));
  Add(new BitsliceStep<XOR<2>>(word_size_, A->Rotl(12), T_[round]->Rotl(7),
                               TA_[round]));
  Add(new CarryStep<ADD<2>>(word_size_, D, F_[round], FD_[round]));
  Add(new CarryStep<ADD<2>>(word_size_, TA_[round], FD_[round], FDTA_[round]));
  Add(new CarryStep<ADD<2>>(word_size_, FDTA_[round], Wp_[round], A_[round]));

  Add(new CarryStep<ADD<2>>(word_size_, G_[round], H, GH_[round]));
  Add(new CarryStep<ADD<2>>(word_size_, GH_[round], T_[round]->Rotl(7),
                            GHT_[round]));
  Add(new CarryStep<ADD<2>>(word_size_, GHT_[round], W_[round], P_[round]));
}

bool Sm3::Callback(std::string function, Characteristic& characteristic,
                   std::mt19937& rng, Logfile& logfile) {
  // logfile << "callback: " << function << std::endl;
  if (function.compare("firstblock") == 0)
    return FirstBlock(characteristic, rng, logfile);

  return Crypto::Callback(function, characteristic, rng, logfile);
}

bool Sm3::FirstBlock(Characteristic& characteristic, std::mt19937& rng,
                     Logfile& logfile) {
  if (num_rounds_ < 16) return false;

#define ROTATELEFT(X, n) (((X) << (n)) | ((X) >> (32 - (n))))

#define P0(x) ((x) ^ ROTATELEFT((x), 9) ^ ROTATELEFT((x), 17))
#define P1(x) ((x) ^ ROTATELEFT((x), 15) ^ ROTATELEFT((x), 23))

#define FF0(x, y, z) ((x) ^ (y) ^ (z))
#define FF1(x, y, z) (((x) & (y)) | ((x) & (z)) | ((y) & (z)))

#define GG0(x, y, z) ((x) ^ (y) ^ (z))
#define GG1(x, y, z) (((x) & (y)) | ((~(x)) & (z)))

  uint32_t IV[8] = {0x7380166F, 0x4914B2B9, 0x172442D7, 0xDA8A0600,
                    0xA96F30BC, 0x163138AA, 0xE38DEE4D, 0xB0FB0E4E};

  uint32_t W[68], W1[64];
  uint32_t HH[8], mask0[8], mask1[8];
  uint32_t SS1, SS2, TT1, TT2, T[64];
  int j;
  Characteristic s = characteristic;

  mask0[3] = s.GetConditionWordMask(
      [](BitCondition bc) { return (bc == BitCondition("0")); },
      GetConditionWordIndex("A", -4));
  mask1[3] = s.GetConditionWordMask(
      [](BitCondition bc) { return (bc == BitCondition("1")); },
      GetConditionWordIndex("A", -4));

  mask0[2] = s.GetConditionWordMask(
      [](BitCondition bc) { return (bc == BitCondition("0")); },
      GetConditionWordIndex("A", -3));
  mask1[2] = s.GetConditionWordMask(
      [](BitCondition bc) { return (bc == BitCondition("1")); },
      GetConditionWordIndex("A", -2));

  mask0[1] = s.GetConditionWordMask(
      [](BitCondition bc) { return (bc == BitCondition("0")); },
      GetConditionWordIndex("A", -2));
  mask1[1] = s.GetConditionWordMask(
      [](BitCondition bc) { return (bc == BitCondition("1")); },
      GetConditionWordIndex("A", -2));

  mask0[0] = s.GetConditionWordMask(
      [](BitCondition bc) { return (bc == BitCondition("0")); },
      GetConditionWordIndex("A", -1));
  mask1[0] = s.GetConditionWordMask(
      [](BitCondition bc) { return (bc == BitCondition("1")); },
      GetConditionWordIndex("A", -1));

  mask0[7] = s.GetConditionWordMask(
      [](BitCondition bc) { return (bc == BitCondition("0")); },
      GetConditionWordIndex("E", -4));
  mask1[7] = s.GetConditionWordMask(
      [](BitCondition bc) { return (bc == BitCondition("1")); },
      GetConditionWordIndex("E", -4));

  mask0[6] = s.GetConditionWordMask(
      [](BitCondition bc) { return (bc == BitCondition("0")); },
      GetConditionWordIndex("E", -3));
  mask1[6] = s.GetConditionWordMask(
      [](BitCondition bc) { return (bc == BitCondition("1")); },
      GetConditionWordIndex("E", -3));

  mask0[5] = s.GetConditionWordMask(
      [](BitCondition bc) { return (bc == BitCondition("0")); },
      GetConditionWordIndex("E", -2));
  mask1[5] = s.GetConditionWordMask(
      [](BitCondition bc) { return (bc == BitCondition("1")); },
      GetConditionWordIndex("E", -2));

  mask0[4] = s.GetConditionWordMask(
      [](BitCondition bc) { return (bc == BitCondition("0")); },
      GetConditionWordIndex("E", -1));
  mask1[4] = s.GetConditionWordMask(
      [](BitCondition bc) { return (bc == BitCondition("1")); },
      GetConditionWordIndex("E", -1));

  /*
   for (j=0;j<8;j++) {
   printf("%08x %08x\n", mask0[j],mask1[j]);
   }
   */

  bool found = false;
  while (!found) {
    for (j = 0; j < 8; j++) {
      HH[j] = IV[j];
    }

    uint32_t A = HH[0];
    uint32_t B = HH[1];
    uint32_t C = HH[2];
    uint32_t D = HH[3];
    uint32_t E = HH[4];
    uint32_t F = HH[5];
    uint32_t G = HH[6];
    uint32_t H = HH[7];

    for (j = 0; j < 16; j++) {
      W[j] = rng();
    }
    for (j = 16; j < 68; j++) {
      W[j] = P1(W[j - 16] ^ W[j - 9] ^ ROTATELEFT(W[j - 3], 15)) ^
             ROTATELEFT(W[j - 13], 7) ^ W[j - 6];
      ;
    }
    for (j = 0; j < 64; j++) {
      W1[j] = W[j] ^ W[j + 4];
    }

    for (j = 0; j < 16; j++) {
      T[j] = 0x79CC4519;
      SS1 = ROTATELEFT((ROTATELEFT(A, 12) + E + ROTATELEFT(T[j], j)), 7);
      SS2 = SS1 ^ ROTATELEFT(A, 12);
      TT1 = FF0(A, B, C) + D + SS2 + W1[j];
      TT2 = GG0(E, F, G) + H + SS1 + W[j];
      D = C;
      C = ROTATELEFT(B, 9);
      B = A;
      A = TT1;
      H = G;
      G = ROTATELEFT(F, 19);
      F = E;
      E = P0(TT2);
    }

    for (j = 16; j < num_rounds_; j++) {
      T[j] = 0x7A879D8A;
      SS1 = ROTATELEFT((ROTATELEFT(A, 12) + E + ROTATELEFT(T[j], j)), 7);
      SS2 = SS1 ^ ROTATELEFT(A, 12);
      TT1 = FF1(A, B, C) + D + SS2 + W1[j];
      TT2 = GG1(E, F, G) + H + SS1 + W[j];
      D = C;
      C = ROTATELEFT(B, 9);
      B = A;
      A = TT1;
      H = G;
      G = ROTATELEFT(F, 19);
      F = E;
      E = P0(TT2);
    }

    HH[0] ^= A;
    HH[1] ^= B;
    HH[2] ^= C;
    HH[3] ^= D;
    HH[4] ^= E;
    HH[5] ^= F;
    HH[6] ^= G;
    HH[7] ^= H;

    // TODO check rotation of chaining values!!!
    HH[2] = ROTATELEFT(HH[2], 23);
    HH[3] = ROTATELEFT(HH[3], 23);
    HH[6] = ROTATELEFT(HH[6], 13);
    HH[7] = ROTATELEFT(HH[7], 13);

    // check masks
    bool res = true;
    for (j = 0; j < 8; j++) {
      res &= (HH[j] & mask0[j]) == 0;
      res &= (HH[j] & mask1[j]) == mask1[j];
    }

    if (!res) continue;

    s.SetWord("A", -4, HH[3], HH[3]);
    s.SetWord("A", -3, HH[2], HH[2]);
    s.SetWord("A", -2, HH[1], HH[1]);
    s.SetWord("A", -1, HH[0], HH[0]);

    s.SetWord("E", -4, HH[7], HH[7]);
    s.SetWord("E", -3, HH[6], HH[6]);
    s.SetWord("E", -2, HH[5], HH[5]);
    s.SetWord("E", -1, HH[4], HH[4]);

    if (s.Update()) {
      found = true;
      characteristic.SetWord("A", -4, HH[3], HH[3]);
      characteristic.SetWord("A", -3, HH[2], HH[2]);
      characteristic.SetWord("A", -2, HH[1], HH[1]);
      characteristic.SetWord("A", -1, HH[0], HH[0]);

      characteristic.SetWord("E", -4, HH[7], HH[7]);
      characteristic.SetWord("E", -3, HH[6], HH[6]);
      characteristic.SetWord("E", -2, HH[5], HH[5]);
      characteristic.SetWord("E", -1, HH[4], HH[4]);

      characteristic.Update();
    } else {
      s = characteristic;
    }
  }
  // characteristic.PrintCharacteristic();

  // TODO store message to  output at the end!!
  printf("\n\n");
  for (j = 0; j < 16; j++) {
    printf("%08x ", W[j]);
    if (j % 8 == 7) printf("\n");
  }
  printf("\n\n");

  return true;
}

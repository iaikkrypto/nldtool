#include "siphash/siphash.h"

#include "bitslice_step.h"
#include "functions.h"
#include "horizontal_condition_word.h"
#include "linear_step.h"
#include "logfile.h"

//#define PROBABILITY_STEP

#ifdef PROBABILITY_STEP
#include "probabilitystep.h"
#endif

const uint64_t Siphash::C64[4] = {0x736f6d6570736575ULL, 0x646f72616e646f6dULL,
                                  0x6c7967656e657261ULL, 0x7465646279746573ULL};

constexpr int Siphash::R64[6];
constexpr int Siphash::R32[6];

void Siphash::AddToOptions(cxxopts::Options& options) {
  options.add_options("SipHash specific")           //
      ("siphash-bs",                                //
       "bitslice flags",                            //
       cxxopts::value<int>()->default_value("-1"),  //
       "FLAG")                                      //
      ("siphash-ls",                                //
       "linear flags",                              //
       cxxopts::value<int>()->default_value("-1"),  //
       "FLAG");
}

Siphash::Siphash(cxxopts::Options& options)
    : Crypto(options["word-size"].as<int>()),
      num_rounds_(options["num-rounds"].as<int>()),
      message_blocks_(options["blocks"].as<int>()),
      bitslice_flags_(options["siphash-bs"].as<int>()),
      linear_flags_(options["siphash-ls"].as<int>()),
      best_prob_(-std::numeric_limits<float>::infinity()) {
  //   flags      bitslice                                linear
  //    1       single num_rounds (prob calc)                  one round
  //    2       2 additions combined (prob calc)          just additions
  //    4       2 additions combined graph (prob calc)    add followed by xor
  //    16      2 additions combined overlap (prob calc)  -
  //    32      3 additions combined overlap              -
  //    256     TBS single num_rounds                          -
  //    512     TBS 2 additions combined                  -

  std::string char_index[] = {"a", "b", "c", "d"};
  Step* step;

  int c = num_rounds_ / SIPHASH_MAXROUNDS;
  int d = num_rounds_ % SIPHASH_MAXROUNDS;

  switch (word_size_) {
    case 64:
      for (int i = 0; i < 6; i++) rot_[i] = R64[i];
      break;
    case 32:
      for (int i = 0; i < 6; i++) rot_[i] = R32[i];
      break;
  }

  if (word_size_ == 64 || word_size_ == 32) {
    //---------------------------------------------------------------------------

    //------------Creating words before compression--------------
    if ((bitslice_flags_ & (two_bit_flags_)) != 0) {
      K2[0] = AddConditionWord("K2", 0, -1, -1, HORIZONTALWORD);
      K2[1] = AddConditionWord("K2", 0, -1, -1, HORIZONTALWORD);

      K[0] = AddReferenceToConditionWord(K2[0], "Ka", 0, 0, 0, MAINWORD, 1);
      K[1] = AddReferenceToConditionWord(K2[1], "Kb", 0, 0, 1, MAINWORD, 1);

      for (int i = 0; i < 4; i++)
        V2[0][i] = AddConditionWord("VTWO", 1, -1, -1, HORIZONTALWORD);
      M2[0] = AddConditionWord("M2", 1, -1, -1, HORIZONTALWORD);

      for (int i = 0; i < 4; i++)
        V[0][i] = AddReferenceToConditionWord(V2[0][i], "V" + char_index[i], 1,
                                              1 + (i / 2), i % 2, MAINWORD, 1);
      M[0] = AddReferenceToConditionWord(M2[0], "M", 1, 1, 2, MAINWORD, 1);
    } else {
      K[0] = AddConditionWord("Ka", 0, 0, 0);
      K[1] = AddConditionWord("Kb", 0, 0, 1);

      for (int i = 0; i < 4; i++)
        V[0][i] = AddConditionWord("V" + char_index[i], 1, 1 + (i / 2), i % 2);
      M[0] = AddConditionWord("M", 1, 1, 2);
    }

    //------------Creatings word for compression--------------

    for (int m = 1; m <= message_blocks_; m++) {
      if ((bitslice_flags_ & (two_bit_flags_)) != 0) {
        for (int j = 1; j <= c; j++) {
          for (int i = 0; i < 4; i++) {
            A2[(m - 1) * c + j + (m - 1) - 1][i] = AddConditionWord(
                "ATWO", (m - 1) * c + j + (m - 1), -1, -1, HORIZONTALWORD);
          }
          for (int i = 0; i < 4; i++) {
            V2[(m - 1) * c + j + (m - 1)][i] = AddConditionWord(
                "VTWO", (m - 1) * c + j + (m - 1) + 1, -1, -1, HORIZONTALWORD);
          }
        }

        for (int j = 1; j <= c; j++) {
          for (int i = 0; i < 4; i++) {
            A[(m - 1) * c + j + (m - 1) - 1][i] = AddReferenceToConditionWord(
                A2[(m - 1) * c + j + (m - 1) - 1][i], "A" + char_index[i],
                (m - 1) * c + j + (m - 1),
                4 * (m - 1) * c + 4 * j + (m - 1) + (i / 2) - 1, i % 2, SUBWORD,
                1);
          }
          for (int i = 0; i < 4; i++) {
            V[(m - 1) * c + j + (m - 1)][i] = AddReferenceToConditionWord(
                V2[(m - 1) * c + j + (m - 1)][i], "V" + char_index[i],
                (m - 1) * c + j + (m - 1) + 1,
                4 * (m - 1) * c + 4 * j + (m - 1) + (i / 2) + 1, i % 2,
                MAINWORD, 1);
          }
        }
      } else {
        for (int j = 1; j <= c; j++) {
          for (int i = 0; i < 4; i++) {
            A[(m - 1) * c + j + (m - 1) - 1][i] = AddConditionWord(
                "A" + char_index[i], (m - 1) * c + j + (m - 1),
                4 * (m - 1) * c + 4 * j + (m - 1) + (i / 2) - 1, i % 2,
                SUBWORD);
          }
          for (int i = 0; i < 4; i++) {
            V[(m - 1) * c + j + (m - 1)][i] = AddConditionWord(
                "V" + char_index[i], (m - 1) * c + j + (m - 1) + 1,
                4 * (m - 1) * c + 4 * j + (m - 1) + (i / 2) + 1, i % 2);
          }
        }
      }

      if (m < message_blocks_) {
        int round_begin = (m - 1) * c + (m - 1) + c;
        int round_end = (m - 1) * c + (m - 1) + c + 1;
        if ((bitslice_flags_ & (two_bit_flags_)) != 0) {
          V2[round_end][0] =
              AddConditionWord("VTWO", round_end + 1, -1, -1, HORIZONTALWORD);
          V2[round_end][1] = V2[round_begin][1];
          V2[round_end][2] = V2[round_begin][2];
          V2[round_end][3] =
              AddConditionWord("VTWO", round_end + 1, -1, -1, HORIZONTALWORD);
          M2[m] = AddConditionWord("M2", round_end, -1, -1, HORIZONTALWORD);

          V[round_end][0] =
              AddReferenceToConditionWord(V2[round_end][0], "Va", round_end + 1,
                                          4 * m * c + 2 + m, 0, MAINWORD, 1);
          V[round_end][1] = V[round_begin][1];
          V[round_end][2] = V[round_begin][2];
          V[round_end][3] =
              AddReferenceToConditionWord(V2[round_end][3], "Vd", round_end + 1,
                                          4 * m * c + 2 + m, 1, MAINWORD, 1);
          M[m] = AddReferenceToConditionWord(M2[m], "M", round_end + 1,
                                             4 * m * c + 2 + m, 2, MAINWORD, 1);
        } else {
          V[round_end][0] =
              AddConditionWord("Va", round_end + 1, 4 * m * c + 2 + m, 0);
          V[round_end][1] = V[round_begin][1];
          V[round_end][2] = V[round_begin][2];
          V[round_end][3] =
              AddConditionWord("Vd", round_end + 1, 4 * m * c + 2 + m, 1);
          M[m] = AddConditionWord("M", round_end + 1, 4 * m * c + 2 + m, 2);
        }
      }
    }

    //------------Creating words after compression--------------
    int round_begin = (message_blocks_ - 1) * c + (message_blocks_ - 1) + c;
    int before_d_rounds =
        (message_blocks_ - 1) * c + (message_blocks_ - 1) + c + 1;
    if ((bitslice_flags_ & (two_bit_flags_)) != 0) {
      V2[before_d_rounds][0] =
          AddConditionWord("VTWO", before_d_rounds, -1, -1, HORIZONTALWORD);
      V2[before_d_rounds][1] = V2[round_begin][1];
      V2[before_d_rounds][2] =
          AddConditionWord("VTWO", before_d_rounds, -1, -1, HORIZONTALWORD);
      V2[before_d_rounds][3] = V2[round_begin][3];

      V[before_d_rounds][0] = AddReferenceToConditionWord(
          V2[before_d_rounds][0], "Va", before_d_rounds + 1,
          4 * message_blocks_ * c + 2 + message_blocks_, 0, MAINWORD, 1);
      V[before_d_rounds][1] = V[round_begin][1];
      V[before_d_rounds][2] = AddReferenceToConditionWord(
          V2[before_d_rounds][2], "Vc", before_d_rounds + 1,
          4 * message_blocks_ * c + 2 + message_blocks_, 1, MAINWORD, 1);
      V[before_d_rounds][3] = V[round_begin][3];
    } else {
      V[before_d_rounds][0] =
          AddConditionWord("Va", before_d_rounds + 1,
                           4 * message_blocks_ * c + 2 + message_blocks_, 0);
      V[before_d_rounds][1] = V[round_begin][1];
      V[before_d_rounds][2] =
          AddConditionWord("Vc", before_d_rounds + 1,
                           4 * message_blocks_ * c + 2 + message_blocks_, 1);
      V[before_d_rounds][3] = V[round_begin][3];
    }

    //------------Creating words for finalization--------------
    if ((bitslice_flags_ & (two_bit_flags_)) != 0) {
      for (int j = 0; j < d; j++) {
        for (int i = 0; i < 4; i++) {
          A2[j + before_d_rounds][i] = AddConditionWord(
              "ATWO", j + before_d_rounds + 1, -1, -1, HORIZONTALWORD);
        }
        for (int i = 0; i < 4; i++) {
          V2[j + before_d_rounds + 1][i] = AddConditionWord(
              "VTWO", j + before_d_rounds + 2, -1, -1, HORIZONTALWORD);
        }
      }

      for (int j = 0; j < d; j++) {
        for (int i = 0; i < 4; i++) {
          A[j + before_d_rounds][i] = AddReferenceToConditionWord(
              A2[j + before_d_rounds][i], "A" + char_index[i],
              j + before_d_rounds + 1,
              4 * j + (i / 2) + message_blocks_ * c * 4 + message_blocks_ + 3,
              i % 2, SUBWORD, 1);
        }
        for (int i = 0; i < 4; i++) {
          V[j + before_d_rounds + 1][i] = AddReferenceToConditionWord(
              V2[j + before_d_rounds + 1][i], "V" + char_index[i],
              j + before_d_rounds + 2,
              4 * j + (i / 2) + message_blocks_ * c * 4 + message_blocks_ + 5,
              i % 2, MAINWORD, 1);
        }
      }
    } else {
      for (int j = 0; j < d; j++) {
        for (int i = 0; i < 4; i++) {
          A[j + before_d_rounds][i] = AddConditionWord(
              "A" + char_index[i], j + before_d_rounds + 1,
              4 * j + (i / 2) + message_blocks_ * c * 4 + message_blocks_ + 3,
              i % 2, SUBWORD);
        }
        for (int i = 0; i < 4; i++) {
          V[j + before_d_rounds + 1][i] = AddConditionWord(
              "V" + char_index[i], j + before_d_rounds + 2,
              4 * j + (i / 2) + message_blocks_ * c * 4 + message_blocks_ + 5,
              i % 2);
        }
      }
    }

    //------------Word for MAC value--------------
    if ((bitslice_flags_ & (two_bit_flags_)) != 0) {
      OUT2 = AddConditionWord("OTWO", d + before_d_rounds + 2, -1, -1,
                              HORIZONTALWORD);
      kNumOutputs = AddReferenceToConditionWord(
          OUT2, "O", d + before_d_rounds + 2,
          4 * d + message_blocks_ * c * 4 + message_blocks_ + 3, 0, MAINWORD,
          1);
    } else {
      kNumOutputs = AddConditionWord(
          "O", d + before_d_rounds + 2,
          4 * d + message_blocks_ * c * 4 + message_blocks_ + 3, 0);
    }

    //---------------------------------------------------------------------------

    //------------Initial operations--------------
    if ((bitslice_flags_ & (two_bit_flags_)) != 0) {
      Add(new BitsliceStep<XORB<2>>(
          word_size_ - 1,
          ConditionWordPtr(new HorizontalConditionWord(C64[0], word_size_)),
          K2[0], V2[0][0]));
      Add(new BitsliceStep<XORB<2>>(
          word_size_ - 1,
          ConditionWordPtr(new HorizontalConditionWord(C64[1], word_size_)),
          K2[1], V2[0][1]));
      Add(new BitsliceStep<XORB<2>>(
          word_size_ - 1,
          ConditionWordPtr(new HorizontalConditionWord(C64[2], word_size_)),
          K2[0], V2[0][2]));
      Add(new BitsliceStep<XORB<3>>(
          word_size_ - 1,
          ConditionWordPtr(new HorizontalConditionWord(C64[3], word_size_)),
          K2[1], M2[0], V2[0][3]));
    }
    if (((bitslice_flags_ & (one_bit_flags_ | one_bit_2_rounds_flags_)) != 0) ||
        (linear_flags_ != 0 && (bitslice_flags_ & (two_bit_flags_)) == 0)) {
      step = Add(new BitsliceStep<XOR<2>>(
          word_size_, ConditionWordPtr(new ConditionWord(C64[0], word_size_)),
          K[0], V[0][0]));
      V[0][0]->SetStepToComputeProbability(step);
      step = Add(new BitsliceStep<XOR<2>>(
          word_size_, ConditionWordPtr(new ConditionWord(C64[1], word_size_)),
          K[1], V[0][1]));
      V[0][1]->SetStepToComputeProbability(step);
      step = Add(new BitsliceStep<XOR<2>>(
          word_size_, ConditionWordPtr(new ConditionWord(C64[2], word_size_)),
          K[0], V[0][2]));
      V[0][2]->SetStepToComputeProbability(step);
      step = Add(new BitsliceStep<XOR<3>>(
          word_size_, ConditionWordPtr(new ConditionWord(C64[3], word_size_)),
          K[1], M[0], V[0][3]));
      V[0][3]->SetStepToComputeProbability(step);
    }

    //------------Operations for message injection--------------
    for (int m = 1; m < message_blocks_; m++) {
      int round_begin = (m - 1) * c + (m - 1) + c;
      int round_end = (m - 1) * c + (m - 1) + c + 1;
      if ((bitslice_flags_ & (two_bit_flags_)) != 0) {
        Add(new BitsliceStep<XORB<2>>(word_size_ - 1, V2[round_begin][0],
                                      M2[m - 1], V2[round_end][0]));
        Add(new BitsliceStep<XORB<2>>(word_size_ - 1, V2[round_begin][3], M2[m],
                                      V2[round_end][3]));
      }
      if (((bitslice_flags_ & (one_bit_flags_ | one_bit_2_rounds_flags_)) !=
           0) ||
          (linear_flags_ != 0 && (bitslice_flags_ & (two_bit_flags_)) == 0)) {
        step = Add(new BitsliceStep<XOR<2>>(word_size_, V[round_begin][0],
                                            M[m - 1], V[round_end][0]));
        V[round_end][0]->SetStepToComputeProbability(step);
        step = Add(new BitsliceStep<XOR<2>>(word_size_, V[round_begin][3], M[m],
                                            V[round_end][3]));
        V[round_end][3]->SetStepToComputeProbability(step);
      }
    }

    //------------Operations before finalization--------------
    if ((bitslice_flags_ & (two_bit_flags_)) != 0) {
      Add(new BitsliceStep<XORB<2>>(word_size_ - 1, V2[round_begin][0],
                                    M2[message_blocks_ - 1],
                                    V2[before_d_rounds][0]));
      Add(new BitsliceStep<XORB<2>>(
          word_size_ - 1, V2[round_begin][2],
          ConditionWordPtr(new HorizontalConditionWord(0xffULL, word_size_)),
          V2[before_d_rounds][2]));
    }

    if (((bitslice_flags_ & (one_bit_flags_ | one_bit_2_rounds_flags_)) != 0) ||
        (linear_flags_ != 0 && (bitslice_flags_ & (two_bit_flags_)) == 0)) {
      step = Add(new BitsliceStep<XOR<2>>(word_size_, V[round_begin][0],
                                          M[message_blocks_ - 1],
                                          V[before_d_rounds][0]));
      V[before_d_rounds][0]->SetStepToComputeProbability(step);
      step = Add(new BitsliceStep<XOR<2>>(
          word_size_, V[round_begin][2],
          ConditionWordPtr(new ConditionWord(0xffULL, word_size_)),
          V[before_d_rounds][2]));
      V[before_d_rounds][0]->SetStepToComputeProbability(step);
    }

    //------------SipRounds for compression--------------
    for (int m = 1; m <= message_blocks_; m++)
      for (int rounds = 0; rounds < c; rounds++) {
        SipRound((m - 1) * c + (m - 1) + rounds);
        if (rounds < c - 1) {
          TwoSipRounds((m - 1) * c + (m - 1) + rounds, false, false, -1);
        } else {
          if (m == message_blocks_)
            TwoSipRounds((m - 1) * c + (m - 1) + rounds, d == 0, true, m - 1);
          else
            TwoSipRounds((m - 1) * c + (m - 1) + rounds, false, false, m - 1);
        }
      }

    //------------SipRounds for finalization--------------
    for (int rounds = 0; rounds < d; rounds++) {
      SipRound(rounds + before_d_rounds);
      TwoSipRounds(rounds + before_d_rounds, d - rounds == 1, false, -1);
    }

    //------------Final XOR--------------
    if ((bitslice_flags_ & (two_bit_flags_)) != 0) {
      Add(new BitsliceStep<XORB<4>>(word_size_ - 1, V2[d + before_d_rounds][0],
                                    V2[d + before_d_rounds][1],
                                    V2[d + before_d_rounds][2],
                                    V2[d + before_d_rounds][3], OUT2));
    }
    if (((bitslice_flags_ & (one_bit_flags_ | one_bit_2_rounds_flags_)) != 0) ||
        (linear_flags_ != 0 && (bitslice_flags_ & (two_bit_flags_)) == 0)) {
      step = Add(new BitsliceStep<XOR<4>>(
          word_size_, V[d + before_d_rounds][0], V[d + before_d_rounds][1],
          V[d + before_d_rounds][2], V[d + before_d_rounds][3], kNumOutputs));
      kNumOutputs->SetStepToComputeProbability(step);
    }
  }
}

void Siphash::SipRound(int round_begin) {
  int round_end = round_begin + 1;
  ConditionWordPtr c0[2];
  ConditionWordPtr c2[2];
  if ((bitslice_flags_ & two_bit_flags_) != 0) {
    SipRoundTwoBitXors(round_begin);
  }
  if ((bitslice_flags_ & 0x100) != 0) {
    SipRoundTwoBitAdds(round_begin);
  }
  if ((bitslice_flags_ & 0x200) != 0) {
    SipRoundTwoBitCombinedAdds(round_begin);
  }
  if ((bitslice_flags_ & (one_bit_flags_ | one_bit_2_rounds_flags_)) != 0) {
    SipRoundXors(round_begin);
  }
  if ((bitslice_flags_ & (1)) != 0) {
    SipRoundAdds(round_begin, c0, c2);
  }
  if ((bitslice_flags_ & 0x12) != 0) {
    SipRoundCombinedAdds(round_begin);
  }
  if ((bitslice_flags_ & 0x4) != 0) {
    SipRoundCombinedAddsGraph(round_begin);
  }
  if (linear_flags_) {
    if (bitslice_flags_ != 1)
      for (int i = 0; i < 2; i++) {
        c0[i] = AddConditionWord("C", -1, -1, -1, CARRYWORD);
        c2[i] = AddConditionWord("C", -1, -1, -1, CARRYWORD);
      }
  }
  if ((linear_flags_ & 1) != 0) {
    switch (word_size_) {
      case 64:
        Add(new LinearStep<
            SIPROUND<R64[0], R64[1], R64[2], R64[3], R64[4], R64[5]>>(
            word_size_, V[round_begin][0], V[round_begin][1], V[round_begin][2],
            V[round_begin][3], c0[0], c0[1], c2[0], c2[1], V[round_end][0],
            V[round_end][1], V[round_end][2], V[round_end][3],
            A[round_begin][0], A[round_begin][1], A[round_begin][2],
            A[round_begin][3]));
        break;
      case 32:
        Add(new LinearStep<
            SIPROUND<R32[0], R32[1], R32[2], R32[3], R32[4], R32[5]>>(
            word_size_, V[round_begin][0], V[round_begin][1], V[round_begin][2],
            V[round_begin][3], c0[0], c0[1], c2[0], c2[1], V[round_end][0],
            V[round_end][1], V[round_end][2], V[round_end][3],
            A[round_begin][0], A[round_begin][1], A[round_begin][2],
            A[round_begin][3]));
        break;
    }
  }
  if ((linear_flags_ & 2) != 0) {
    switch (word_size_) {
      case 64:
        Add(new LinearStep<
            ADDSLIN<R64[0], R64[1], R64[2], R64[3], R64[4], R64[5]>>(
            word_size_, V[round_begin][0], V[round_begin][1], V[round_begin][2],
            V[round_begin][3], A[round_begin][3], A[round_begin][1], c0[0],
            c0[1], c2[0], c2[1], A[round_begin][0], V[round_end][0],
            A[round_begin][2], V[round_end][2]));
        break;
      case 32:
        Add(new LinearStep<
            ADDSLIN<R32[0], R32[1], R32[2], R32[3], R32[4], R32[5]>>(
            word_size_, V[round_begin][0], V[round_begin][1], V[round_begin][2],
            V[round_begin][3], A[round_begin][3], A[round_begin][1], c0[0],
            c0[1], c2[0], c2[1], A[round_begin][0], V[round_end][0],
            A[round_begin][2], V[round_end][2]));
        break;
    }
  }
  if ((linear_flags_ & 4) != 0) {
    Add(new LinearStep<ADDXORLIN<R64[1]>>(
        word_size_, V[round_begin][0], V[round_begin][1], c0[0],
        A[round_begin][0], A[round_begin][1]));
    Add(new LinearStep<ADDXORLIN<R64[5]>>(
        word_size_, A[round_begin][0]->Rotl(rot_[0]), A[round_begin][3], c0[1],
        V[round_end][0], V[round_end][3]));
    Add(new LinearStep<ADDXORLIN<R64[4]>>(
        word_size_, V[round_begin][2], V[round_begin][3], c2[0],
        A[round_begin][2], A[round_begin][3]));
    Add(new LinearStep<ADDXORLIN<R64[2]>>(
        word_size_, A[round_begin][2], A[round_begin][1], c2[1],
        V[round_end][2]->Rotr(rot_[3]), V[round_end][1]));
  }
}

void Siphash::SipRoundXors(int round_begin) {
  Step* step;
  int round_end = round_begin + 1;
  step =
      Add(new BitsliceStep<XOR<2>>(word_size_, V[round_begin][1]->Rotl(rot_[1]),
                                   A[round_begin][0], A[round_begin][1]));
  A[round_begin][1]->SetStepToComputeProbability(step);
  step =
      Add(new BitsliceStep<XOR<2>>(word_size_, V[round_begin][3]->Rotl(rot_[4]),
                                   A[round_begin][2], A[round_begin][3]));
  A[round_begin][3]->SetStepToComputeProbability(step);
  step = Add(new BitsliceStep<XOR<2>>(
      word_size_, A[round_begin][1]->Rotl(rot_[2]),
      V[round_end][2]->Rotr(rot_[3]), V[round_end][1]));
  V[round_end][1]->SetStepToComputeProbability(step);
  step =
      Add(new BitsliceStep<XOR<2>>(word_size_, A[round_begin][3]->Rotl(rot_[5]),
                                   V[round_end][0], V[round_end][3]));
  V[round_end][3]->SetStepToComputeProbability(step);
}

void Siphash::SipRoundTwoBitXors(int round_begin) {
  int round_end = round_begin + 1;
  Add(new BitsliceStep<XORB<2>>(word_size_ - 1,
                                V2[round_begin][1]->Rotl(rot_[1]),
                                A2[round_begin][0], A2[round_begin][1]));
  Add(new BitsliceStep<XORB<2>>(word_size_ - 1,
                                V2[round_begin][3]->Rotl(rot_[4]),
                                A2[round_begin][2], A2[round_begin][3]));

  Add(new BitsliceStep<XORB<2>>(
      word_size_ - 1, A2[round_begin][1]->Rotl(rot_[2]),
      V2[round_end][2]->Rotr(rot_[3]), V2[round_end][1]));
  Add(new BitsliceStep<XORB<2>>(word_size_ - 1,
                                A2[round_begin][3]->Rotl(rot_[5]),
                                V2[round_end][0], V2[round_end][3]));
}

void Siphash::SipRoundAdds(int round_begin, ConditionWordPtr* c0,
                           ConditionWordPtr* c2) {
  Step* step;
  int round_end = round_begin + 1;
  for (int i = 0; i < 2; i++) {
    c0[i] = AddConditionWord("C", -1, -1, -1, CARRYWORD);
    c2[i] = AddConditionWord("C", -1, -1, -1, CARRYWORD);
  }
  step = Add(new BitsliceStep<ADD<2>>(word_size_, V[round_begin][0],
                                      V[round_begin][1], c0[0]->Shl(1),
                                      A[round_begin][0], c0[0]));
  A[round_begin][0]->SetStepToComputeProbability(step);
  step = Add(new BitsliceStep<ADD<2>>(word_size_, V[round_begin][2],
                                      V[round_begin][3], c2[0]->Shl(1),
                                      A[round_begin][2], c2[0]));
  A[round_begin][2]->SetStepToComputeProbability(step);
  step = Add(new BitsliceStep<ADD<2>>(
      word_size_, A[round_begin][0]->Rotl(rot_[0]), A[round_begin][3],
      c0[1]->Shl(1), V[round_end][0], c0[1]));
  V[round_end][0]->SetStepToComputeProbability(step);
  step = Add(new BitsliceStep<ADD<2>>(word_size_, A[round_begin][1],
                                      A[round_begin][2], c2[1]->Shl(1),
                                      V[round_end][2]->Rotr(rot_[3]), c2[1]));
  V[round_end][2]->SetStepToComputeProbability(step);

#ifdef PROBABILITY_STEP
  Add(new ProbabilityStep<ADD<2>>(word_size_, V[round_begin][0],
                                  V[round_begin][1], c0[0]->Shl(1),
                                  A[round_begin][0], c0[0]));
  Add(new ProbabilityStep<ADD<2>>(word_size_, V[round_begin][2],
                                  V[round_begin][3], c2[0]->Shl(1),
                                  A[round_begin][2], c2[0]));
  Add(new ProbabilityStep<ADD<2>>(word_size_, A[round_begin][0]->Rotl(rot_[0]),
                                  A[round_begin][3], c0[1]->Shl(1),
                                  V[round_end][0], c0[1]));
  Add(new ProbabilityStep<ADD<2>>(word_size_, A[round_begin][1],
                                  A[round_begin][2], c2[1]->Shl(1),
                                  V[round_end][2]->Rotr(rot_[3]), c2[1]));
#endif
}

void Siphash::SipRoundCombinedAdds(int round_begin) {
  Step* step;
  int round_end = round_begin + 1;
  ConditionWordPtr c0o = AddConditionWord("C", -1, -1, -1, CARRYWORD, 2);
  ConditionWordPtr c2o = AddConditionWord("C", -1, -1, -1, CARRYWORD, 2);
  ConditionWordPtr c0i = c0o->Rotl(1)->SetZero(0, 1, 0)->SetZero(rot_[0], 1, 1);
  ConditionWordPtr c2i = c2o->Shl(1);

  step = Add(new BitsliceStep<ADD2ADD2>(
      word_size_, V[round_begin][0]->Rotl(rot_[0]),
      V[round_begin][1]->Rotl(rot_[0]), A[round_begin][3], c0i,
      A[round_begin][0]->Rotr(rot_[0]), V[round_end][0], c0o));
  V[round_end][0]->SetStepToComputeProbability(step);
  step->SetProbabilityMethod(CYCLICGRAPH);

  step = Add(new BitsliceStep<ADD2ADD2>(
      word_size_, V[round_begin][2], V[round_begin][3], A[round_begin][1], c2i,
      A[round_begin][2], V[round_end][2]->Rotr(rot_[3]), c2o));
  V[round_end][2]->SetStepToComputeProbability(step);
  step->SetProbabilityMethod(CYCLICGRAPH);
#ifdef PROBABILITY_STEP
  step = Add(new ProbabilityStep<ADD2ADD2>(
      word_size_, V[round_begin][0]->Rotl(rot_[0]),
      V[round_begin][1]->Rotl(rot_[0]), A[round_begin][3], c0i,
      A[round_begin][0]->Rotr(rot_[0]), V[round_end][0], c0o));
  step->SetProbabilityMethod(CYCLICGRAPH);

  step = Add(new ProbabilityStep<ADD2ADD2>(
      word_size_, V[round_begin][2], V[round_begin][3], A[round_begin][1], c2i,
      A[round_begin][2], V[round_end][2]->Rotr(rot_[3]), c2o));
  step->SetProbabilityMethod(CYCLICGRAPH);
#endif
}

void Siphash::SipRoundCombinedAddsGraph(int round_begin) {
  // TODO: remove graphstep functionality of sipcrypto
  /*Step* step;
  int round_end = round_begin + 1;
  ConditionWordPtr c0o = AddConditionWord("C", -1, -1, -1, CARRYWORD, 2);
  ConditionWordPtr c2o = AddConditionWord("C", -1, -1, -1, CARRYWORD, 2);
  ConditionWordPtr c0i = c0o->Rotl(1)->SetZero(0, 1, 0)->SetZero(rot_[0], 1, 1);
  ConditionWordPtr c2i = c2o->Shl(1);
  step = Add(
      new GraphStep<ADD2ADD2>(word_size_, V[round_begin][0]->Rotl(rot_[0]),
          V[round_begin][1]->Rotl(rot_[0]), A[round_begin][3], c0i,
          A[round_begin][0]->Rotr(rot_[0]), V[round_end][0], c0o));
  V[round_end][0]->SetStepToComputeProbability(step);
  step->SetProbabilityMethod(CYCLICGRAPH);
  step = Add(
      new GraphStep<ADD2ADD2>(word_size_, V[round_begin][2], V[round_begin][3],
A[round_begin][1], c2i, A[round_begin][2], V[round_end][2]->Rotr(rot_[3]),
c2o)); V[round_end][2]->SetStepToComputeProbability(step);
  step->SetProbabilityMethod(CYCLICGRAPH);
#ifdef PROBABILITY_STEP
  step = Add(
      new ProbabilityStep<ADD2ADD2>(word_size_,
V[round_begin][0]->Rotl(rot_[0]), V[round_begin][1]->Rotl(rot_[0]),
A[round_begin][3], c0i, A[round_begin][0]->Rotr(rot_[0]), V[round_end][0],
c0o)); step->SetProbabilityMethod(CYCLICGRAPH); step = Add( new
ProbabilityStep<ADD2ADD2>(word_size_, V[round_begin][2], V[round_begin][3],
A[round_begin][1], c2i, A[round_begin][2], V[round_end][2]->Rotr(rot_[3]),
c2o)); step->SetProbabilityMethod(CYCLICGRAPH); #endif
   */
}

void Siphash::SipRoundTwoBitAdds(int round_begin) {
  int round_end = round_begin + 1;
  ConditionWordPtr c0[2];
  ConditionWordPtr c2[2];
  for (int i = 0; i < 2; i++) {
    c0[i] = AddConditionWord("C", -1, -1, -1, CARRYWORD);
    c2[i] = AddConditionWord("C", -1, -1, -1, CARRYWORD);
  }
  Add(new BitsliceStep<ADDB<2>>(word_size_ - 1, V2[round_begin][0],
                                V2[round_begin][1], c0[0]->Shl(1),
                                A2[round_begin][0], c0[0], c0[0]->Shr(1)));
  Add(new BitsliceStep<ADDB<2>>(word_size_ - 1, V2[round_begin][2],
                                V2[round_begin][3], c2[0]->Shl(1),
                                A2[round_begin][2], c2[0], c2[0]->Shr(1)));
  Add(new BitsliceStep<ADDB<2>>(
      word_size_ - 1, A2[round_begin][0]->Rotl(rot_[0]), A2[round_begin][3],
      c0[1]->Shl(1), V2[round_end][0], c0[1], c0[1]->Shr(1)));
  Add(new BitsliceStep<ADDB<2>>(
      word_size_ - 1, A2[round_begin][1], A2[round_begin][2], c2[1]->Shl(1),
      V2[round_end][2]->Rotr(rot_[3]), c2[1], c2[1]->Shr(1)));
}

void Siphash::SipRoundTwoBitCombinedAdds(int round_begin) {
  int round_end = round_begin + 1;
  ConditionWordPtr c0mo = AddConditionWord("C", -1, -1, -1, CARRYWORD, 2);
  ConditionWordPtr c2mo = AddConditionWord("C", -1, -1, -1, CARRYWORD, 2);
  ConditionWordPtr c0i =
      c0mo->Rotl(1)->SetZero(0, 1, 0)->SetZero(rot_[0], 1, 1);
  ConditionWordPtr c2i = c2mo->Shl(1);
  ConditionWordPtr c0o = c0mo->Rotr(1);
  ConditionWordPtr c2o = c2mo->Rotr(1);
  Add(new BitsliceStep<ADD2ADD2B>(
      word_size_ - 1 - rot_[0], V2[round_begin][0]->Rotl(rot_[0]),
      V2[round_begin][1]->Rotl(rot_[0]), A2[round_begin][3], c0i,
      A2[round_begin][0]->Rotr(rot_[0]), V2[round_end][0], c0mo, c0o));
  Add(new BitsliceStep<ADD2ADD2B>(
      rot_[0] - 1, V2[round_begin][0]->Rotl(rot_[0])->Shr(rot_[0]),
      V2[round_begin][1]->Rotl(rot_[0])->Shr(rot_[0]),
      A2[round_begin][3]->Shr(rot_[0]), c0i->Shr(rot_[0]),
      A2[round_begin][0]->Rotr(rot_[0])->Shr(rot_[0]),
      V2[round_end][0]->Shr(rot_[0]), c0mo->Shr(rot_[0]), c0o->Shr(rot_[0])));
  Add(new BitsliceStep<ADD2ADD2B>(word_size_ - 1, V2[round_begin][2],
                                  V2[round_begin][3], A2[round_begin][1], c2i,
                                  A2[round_begin][2],
                                  V2[round_end][2]->Rotr(rot_[3]), c2mo, c2o));
}

void Siphash::TwoSipRounds(int current_round, bool no_more_round,
                           bool transition_c_to_d, int message_pos) {
  if ((bitslice_flags_ & 0x10) != 0) {
    if (no_more_round == false) {
      if (message_pos == -1) {
        int round_end = current_round + 1;
        ConditionWordPtr c0o = AddConditionWord("C", -1, -1, -1, CARRYWORD, 2);
        ConditionWordPtr c2o = AddConditionWord("C", -1, -1, -1, CARRYWORD, 2);
        ConditionWordPtr c0i = c0o->Shl(1);
        ConditionWordPtr c2i =
            c2o->Rotl(1)->SetZero(0, 1, 0)->SetZero(rot_[3], 1, 1);
        Add(new BitsliceStep<ADD2ADD2>(
            word_size_, A[current_round][0]->Rotl(rot_[0]), A[current_round][3],
            V[round_end][1], c0i, V[round_end][0], A[round_end][0], c0o));
        Add(new BitsliceStep<ADD2ADD2>(
            word_size_, A[current_round][2]->Rotl(rot_[3]),
            A[current_round][1]->Rotl(rot_[3]), V[round_end][3], c2i,
            V[round_end][2], A[round_end][2], c2o));
      } else {
        int round_p1 = current_round + 1;
        int round_p2 = current_round + 2;
        ConditionWordPtr c0o = AddConditionWord("C", -1, -1, -1, CARRYWORD, 2);
        ConditionWordPtr c2o = AddConditionWord("C", -1, -1, -1, CARRYWORD, 2);
        ConditionWordPtr c0i = c0o->Shl(1);
        ConditionWordPtr c2i =
            c2o->Rotl(1)->SetZero(0, 1, 0)->SetZero(rot_[3], 1, 1);
        Add(new BitsliceStep<ADD2ADD2XOR>(
            word_size_, A[current_round][0]->Rotl(rot_[0]), A[current_round][3],
            M[message_pos], V[round_p2][1], c0i, V[round_p1][0], V[round_p2][0],
            A[round_p2][0], c0o));
        if (transition_c_to_d == true)
          Add(new BitsliceStep<ADD2ADD2XOR>(
              word_size_, A[current_round][2]->Rotl(rot_[3]),
              A[current_round][1]->Rotl(rot_[3]),
              ConditionWordPtr(new ConditionWord(0xffULL, word_size_)),
              V[round_p2][3], c2i, V[round_p1][2], V[round_p2][2],
              A[round_p2][2], c2o));
        else
          Add(new BitsliceStep<ADD2ADD2>(
              word_size_, A[current_round][2]->Rotl(rot_[3]),
              A[current_round][1]->Rotl(rot_[3]), V[round_p2][3], c2i,
              V[round_p1][2], A[round_p2][2], c2o));
      }
    }
  }
  if ((bitslice_flags_ & 0x20) != 0) {
    if (no_more_round == false) {
      int round_p1 = current_round + 1;
      int round_p2 = current_round + 2;
      int round_p3 = current_round + 3;

      if (message_pos == -1) {
        ConditionWordPtr c0o = AddConditionWord("C", -1, -1, -1, CARRYWORD, 3);
        ConditionWordPtr c2o = AddConditionWord("C", -1, -1, -1, CARRYWORD, 3);
        ConditionWordPtr c0i =
            c0o->Rotl(1)->SetZero(rot_[0], 1, 0)->SetZero(0, 2, 1);
        ConditionWordPtr c2i =
            c2o->Rotl(1)->SetZero(rot_[3], 2, 0)->SetZero(0, 1, 2);
        Add(new BitsliceStep<ADD2U1U1ROT>(
            word_size_, V[current_round][0]->Rotl(rot_[0]),
            V[current_round][1]->Rotl(rot_[0]), A[current_round][3],
            V[round_p1][1], c0i, A[current_round][0]->Rotr(rot_[0]),
            V[round_p1][0], A[round_p1][0], c0o));
        Add(new BitsliceStep<ADD2U1U1ROT>(
            word_size_, V[current_round][2]->Rotl(rot_[3]),
            V[current_round][3]->Rotl(rot_[3]),
            A[current_round][1]->Rotl(rot_[3]), V[round_p1][3], c2i,
            A[current_round][2]->Rotr(rot_[3]), V[round_p1][2], A[round_p1][2],
            c2o));
        c0o = AddConditionWord("C", -1, -1, -1, CARRYWORD, 3);
        c2o = AddConditionWord("C", -1, -1, -1, CARRYWORD, 3);
        c0i = c0o->Rotl(1)->SetZero(rot_[0], 2, 0)->SetZero(0, 1, 2);
        c2i = c2o->Rotl(1)->SetZero(rot_[3], 1, 0)->SetZero(0, 2, 1);
        Add(new BitsliceStep<ADD2U1U1ROT>(
            word_size_, A[current_round][0], A[current_round][3]->Rotl(rot_[0]),
            V[round_p1][1]->Rotl(rot_[0]), A[round_p1][3], c0i,
            V[round_p1][0]->Rotr(rot_[0]), A[round_p1][0]->Rotr(rot_[0]),
            V[round_p2][0], c0o));
        Add(new BitsliceStep<ADD2U1U1ROT>(
            word_size_, A[current_round][2]->Rotl(rot_[3]),
            A[current_round][1]->Rotl(rot_[3]), V[round_p1][3], A[round_p1][1],
            c2i, V[round_p1][2], A[round_p1][2], V[round_p2][2]->Rotr(rot_[3]),
            c2o));
      } else {
        ConditionWordPtr c0o = AddConditionWord("C", -1, -1, -1, CARRYWORD, 3);
        ConditionWordPtr c2o = AddConditionWord("C", -1, -1, -1, CARRYWORD, 3);
        ConditionWordPtr c0i =
            c0o->Rotl(1)->SetZero(rot_[0], 1, 0)->SetZero(0, 2, 1);
        ConditionWordPtr c2i =
            c2o->Rotl(1)->SetZero(rot_[3], 2, 0)->SetZero(0, 1, 2);
        Add(new BitsliceStep<ADD2U1XORU1ROT>(
            word_size_, V[current_round][0]->Rotl(rot_[0]),
            V[current_round][1]->Rotl(rot_[0]), A[current_round][3],
            M[message_pos], V[round_p2][1], c0i,
            A[current_round][0]->Rotr(rot_[0]), V[round_p1][0], V[round_p2][0],
            A[round_p2][0], c0o));
        if (transition_c_to_d == true)
          Add(new BitsliceStep<ADD2U1XORU1ROT>(
              word_size_, V[current_round][2]->Rotl(rot_[3]),
              V[current_round][3]->Rotl(rot_[3]),
              A[current_round][1]->Rotl(rot_[3]),
              ConditionWordPtr(new ConditionWord(0xffULL, word_size_)),
              V[round_p2][3], c2i, A[current_round][2]->Rotr(rot_[3]),
              V[round_p1][2], V[round_p2][2], A[round_p2][2], c2o));
        else
          Add(new BitsliceStep<ADD2U1U1ROT>(
              word_size_, V[current_round][2]->Rotl(rot_[3]),
              V[current_round][3]->Rotl(rot_[3]),
              A[current_round][1]->Rotl(rot_[3]), V[round_p2][3], c2i,
              A[current_round][2]->Rotr(rot_[3]), V[round_p1][2],
              A[round_p2][2], c2o));
        c0o = AddConditionWord("C", -1, -1, -1, CARRYWORD, 3);
        c2o = AddConditionWord("C", -1, -1, -1, CARRYWORD, 3);
        c0i = c0o->Rotl(1)->SetZero(rot_[0], 2, 0)->SetZero(0, 1, 2);
        c2i = c2o->Rotl(1)->SetZero(rot_[3], 1, 0)->SetZero(0, 2, 1);
        Add(new BitsliceStep<ADD2XORU1U1ROT>(
            word_size_, A[current_round][0], A[current_round][3]->Rotl(rot_[0]),
            M[message_pos]->Rotl(rot_[0]), V[round_p2][1]->Rotl(rot_[0]),
            A[round_p2][3], c0i, V[round_p1][0]->Rotl(rot_[0]),
            V[round_p2][0]->Rotl(rot_[0]), A[round_p2][0]->Rotr(rot_[0]),
            V[round_p3][0], c0o));
        if (transition_c_to_d == true)
          Add(new BitsliceStep<ADD2XORU1U1ROT>(
              word_size_, A[current_round][2]->Rotl(rot_[3]),
              A[current_round][1]->Rotl(rot_[3]),
              ConditionWordPtr(new ConditionWord(0xffULL, word_size_)),
              V[round_p2][3], A[round_p2][1], c2i, V[round_p1][2],
              V[round_p2][2], A[round_p2][2], V[round_p3][2]->Rotr(rot_[3]),
              c2o));
        else
          Add(new BitsliceStep<ADD2U1U1ROT>(
              word_size_, A[current_round][2]->Rotl(rot_[3]),
              A[current_round][1]->Rotl(rot_[3]), V[round_p2][3],
              A[round_p2][1], c2i, V[round_p2][2], A[round_p2][2],
              V[round_p3][2]->Rotr(rot_[3]), c2o));
      }
    }
  }
}

bool Siphash::Callback(std::string function, Characteristic& characteristic,
                       std::mt19937& rng, Logfile& logfile) {
  if (function.empty())
    return true;
  else if (function.compare("found") == 0) {
    float prob = characteristic.GetProbabilitySum();
    if (prob > best_prob_) {
      logfile << "found new characteristic" << std::endl;
      logfile << "prob: " << prob << std::endl;
      characteristic.WriteCharacteristic(logfile);
      best_prob_ = prob;
    }
    return true;
  }
  assert(false);
  return false;
}

//----------------------------- ADD Classes ------------------------------------
class Siphash::ADD2U1U1ROT : public F {
 public:
  static constexpr char kName[] = "ADD2U1U1ROT";
  static const int kNumInputs = 5;
  static const int kNumOutputs = 4;
  static constexpr int Symmetry(int i) { return (i < 2) ? 0 : i; }
  static constexpr int Bitsize(int i) { return (i == 4 || i == 8) ? 3 : 1; }
  static const int kPrevState = 4;
  static const int kNextState = 8;
  static const int kStateSize = 64;
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    const int ci = kNumInputs - 1;
    const int s1 = 0;
    const int s2 = 1;
    const int s3 = 2;
    const int co = 3;
    y[s1] = x[0] + x[1] + ((x[ci]) & 1);
    y[co] = (y[s1] & 2) >> 1;
    y[s1] &= 1;
    y[s2] = y[s1] + x[2] + ((x[ci] >> 1) & 1);
    y[co] |= y[s2] & 2;
    y[s2] &= 1;
    y[s3] = y[s2] + x[3] + ((x[ci] >> 2) & 1);
    y[co] |= (y[s3] & 2) << 1;
    y[s3] &= 1;
  }
};

constexpr char Siphash::ADD2U1U1ROT::kName[];

class Siphash::ADD2U1XORU1ROT : public F {
 public:
  static constexpr char kName[] = "ADD2U1XORU1ROT";
  static const int kNumInputs = 6;
  static const int kNumOutputs = 5;
  static constexpr int Symmetry(int i) { return (i < 2) ? 0 : i; }
  static constexpr int Bitsize(int i) { return (i == 5 || i == 10) ? 3 : 1; }
  static const int kPrevState = 5;
  static const int kNextState = 10;
  static const int kStateSize = 64;
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    const int ci = kNumInputs - 1;
    const int s1 = 0;
    const int s2 = 1;
    const int sx2 = 2;
    const int s3 = 3;
    const int co = 4;
    y[s1] = x[0] + x[1] + ((x[ci]) & 1);
    y[co] = (y[s1] & 2) >> 1;
    y[s1] &= 1;
    y[s2] = y[s1] + x[2] + ((x[ci] >> 1) & 1);
    y[co] |= y[s2] & 2;
    y[s2] &= 1;
    y[sx2] = y[s2] ^ x[3];
    y[s3] = y[sx2] + x[4] + ((x[ci] >> 2) & 1);
    y[co] |= (y[s3] & 2) << 1;
    y[s3] &= 1;
  }
};

constexpr char Siphash::ADD2U1XORU1ROT::kName[];

class Siphash::ADD2XORU1U1ROT : public F {
 public:
  static constexpr char kName[] = "ADD2XORU1U1ROT";
  static const int kNumInputs = 6;
  static const int kNumOutputs = 5;
  static constexpr int Symmetry(int i) { return (i < 2) ? 0 : i; }
  static constexpr int Bitsize(int i) { return (i == 5 || i == 10) ? 3 : 1; }
  static const int kPrevState = 5;
  static const int kNextState = 10;
  static const int kStateSize = 64;
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    const int ci = kNumInputs - 1;
    const int s1 = 0;
    const int sx1 = 1;
    const int s2 = 2;
    const int s3 = 3;
    const int co = 4;
    y[s1] = x[0] + x[1] + ((x[ci]) & 1);
    y[co] = (y[s1] & 2) >> 1;
    y[s1] &= 1;
    y[sx1] = y[s1] ^ x[2];
    y[s2] = y[sx1] + x[3] + ((x[ci] >> 1) & 1);
    y[co] |= y[s2] & 2;
    y[s2] &= 1;
    y[s3] = y[s2] + x[4] + ((x[ci] >> 2) & 1);
    y[co] |= (y[s3] & 2) << 1;
    y[s3] &= 1;
  }
};

constexpr char Siphash::ADD2XORU1U1ROT::kName[];

class Siphash::ADD2ADD2 : public F {
 public:
  static constexpr char kName[] = "ADD2ADD2";
  static const int kNumInputs = 4;
  static const int kNumOutputs = 3;
  static constexpr int Symmetry(int i) { return (i < 2) ? 0 : i; }
  static constexpr int Bitsize(int i) { return (i == 3 || i == 6) ? 2 : 1; }
  static const int kPrevState = 3;
  static const int kNextState = 6;
  static const int kStateSize = 16;
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    const int ci = kNumInputs - 1;
    const int sb = 0;
    const int sa = 1;
    const int co = 2;
    y[sb] = x[0] + x[1] + ((x[ci] >> 1) & 1);
    y[co] = y[sb] & 2;
    y[sb] &= 1;
    y[sa] = y[sb] + x[2] + (x[ci] & 1);
    y[co] |= (y[sa] >> 1) & 1;
    y[sa] &= 1;
  }
};

constexpr char Siphash::ADD2ADD2::kName[];

class Siphash::ADD2ADD2XOR : public F {
 public:
  static constexpr char kName[] = "ADD2ADD2XOR";
  static const int kNumInputs = 5;
  static const int kNumOutputs = 4;
  static constexpr int Symmetry(int i) { return (i < 2) ? 0 : i; }
  static constexpr int Bitsize(int i) { return (i == 4 || i == 8) ? 2 : 1; }
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    const int ci = kNumInputs - 1;
    const int s1 = 0;
    const int s1xor = 1;
    const int s2 = 2;
    const int co = 3;
    y[s1] = x[0] + x[1] + ((x[ci] >> 1) & 1);
    y[co] = y[s1] & 2;
    y[s1] &= 1;
    y[s1xor] = y[s1] ^ x[2];
    y[s2] = y[s1xor] + x[3] + (x[ci] & 1);
    y[co] |= (y[s2] >> 1) & 1;
    y[s2] &= 1;
  }
};

constexpr char Siphash::ADD2ADD2XOR::kName[];

class Siphash::ADD2ADD2B : public F {
 public:
  static constexpr char kName[] = "ADD2ADD2B";
  static const int kNumInputs = 4;
  static const int kNumOutputs = 4;
  static constexpr int Symmetry(int i) { return (i < 2) ? 0 : i; }
  static constexpr int Bitsize(int i) { return 2; }
  template <class T>
  static void f(const T x[kNumInputs], T y[kNumOutputs]) {
    const int ci = kNumInputs - 1;
    const int sb = 0;
    const int sa = 1;
    const int cm = 2;
    const int co = 3;
    y[cm] = ((x[0] & 1) + (x[1] & 1) + ((x[ci] >> 1) & 1)) & 2;
    y[sb] = x[0] + x[1] + ((x[ci] >> 1) & 1);
    y[co] = (y[sb] >> 1) & 2;
    y[sb] &= 3;
    y[cm] |= ((y[sb] & 1) + (x[2] & 1) + (x[ci] & 1)) >> 1;
    y[sa] = y[sb] + x[2] + (x[ci] & 1);
    y[co] |= (y[sa] >> 2) & 1;
    y[sa] &= 3;
  }
};

constexpr char Siphash::ADD2ADD2B::kName[];

template <int R0, int R1, int R2, int R3, int R4, int R5>
class Siphash::SIPROUND : public F {
 public:
  static constexpr char kName[] = "SIPROUND";
  static const int kNumInputs = 8;
  static const int kNumOutputs = 8;
  template <class T>
  static void f(int word_size, const T x[kNumInputs], T y[kNumOutputs]) {
    const T *c0, *c2;
    T* a;
    c0 = x + 4;
    c2 = x + 6;
    a = y + 4;
    a[0] = x[0] ^ x[1] ^ nldtool::Shl(c0[0], 1, word_size);
    a[2] = x[2] ^ x[3] ^ nldtool::Shl(c2[0], 1, word_size);
    a[1] = a[0] ^ nldtool::Rotl(x[1], R1, word_size);
    a[3] = a[2] ^ nldtool::Rotl(x[3], R4, word_size);
    y[0] = nldtool::Rotl(a[0], R0, word_size) ^ a[3] ^
           nldtool::Shl(c0[1], 1, word_size);
    y[2] = a[2] ^ a[1] ^ nldtool::Shl(c2[1], 1, word_size);
    y[1] = nldtool::Rotl(a[1], R2, word_size) ^ y[2];
    y[3] = nldtool::Rotl(a[3], R5, word_size) ^ y[0];
    y[2] = nldtool::Rotl(y[2], R3, word_size);
  }
};

template <int R0, int R1, int R2, int R3, int R4, int R5>
constexpr char Siphash::SIPROUND<R0, R1, R2, R3, R4, R5>::kName[];

template <int R0, int R1, int R2, int R3, int R4, int R5>
class Siphash::ADDSLIN : public F {
 public:
  static constexpr char kName[] = "ADDSLIN";
  static const int kNumInputs = 10;
  static const int kNumOutputs = 4;

  template <class T>
  static void f(int word_size, const T x[kNumInputs], T y[kNumOutputs]) {
    const T *c0, *c2;
    c0 = x + 6;
    c2 = x + 8;
    y[0] = x[0] ^ x[1] ^ nldtool::Shl(c0[0], 1, word_size);
    y[2] = x[2] ^ x[3] ^ nldtool::Shl(c2[0], 1, word_size);
    y[1] = nldtool::Rotl(y[0], R0, word_size) ^ x[4] ^
           nldtool::Shl(c0[1], 1, word_size);
    y[3] = nldtool::Rotl(y[2] ^ x[5] ^ nldtool::Shl(c2[1], 1, word_size), R3,
                         word_size);
  }
};

template <int R0, int R1, int R2, int R3, int R4, int R5>
constexpr char Siphash::ADDSLIN<R0, R1, R2, R3, R4, R5>::kName[];

template <int R>
class Siphash::ADDXORLIN : public F {
 public:
  static constexpr char kName[] = "ADDXORLIN";
  static const int kNumInputs = 3;
  static const int kNumOutputs = 2;
  template <class T>
  static void f(int word_size, const T x[kNumInputs], T y[kNumOutputs]) {
    const int c = kNumInputs - 1;
    const int o = 0;
    y[o] = x[0] ^ x[1] ^ nldtool::Shl(x[c], 1, word_size);
    y[o + 1] = y[o] ^ nldtool::Rotl(x[1], R, word_size);
  }
};

template <int R>
constexpr char Siphash::ADDXORLIN<R>::kName[];

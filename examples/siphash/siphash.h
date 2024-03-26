#ifndef SIPHASH_H_
#define SIPHASH_H_

#include "crypto.h"

#define SIPHASH_MAXROUNDS 9
#define SIPHASH_MAX_MSG_BLOCKS 32

/*! \class Siphash
 *  \brief Implementation of the SipHash crypto function.
 *
 *  https://131002.net/siphash
 */
class Siphash : public Crypto {
 public:
  static void AddToOptions(cxxopts::Options &options);
  Siphash(cxxopts::Options &options);
  class ADD2ADD2;
  class ADD2ADD2XOR;
  class ADD2ADD2B;
  class ADD2U1U1ROT;
  class ADD2U1XORU1ROT;
  class ADD2XORU1U1ROT;
  template <int R0, int R1, int R2, int R3, int R4, int R5>
  class SIPROUND;
  template <int R0, int R1, int R2, int R3, int R4, int R5>
  class ADDSLIN;
  template <int R>
  class ADDXORLIN;
  static const uint64_t C64[4];
  static constexpr int R64[6] = {32, 13, 17, 32, 16, 21};
  static constexpr int R32[6] = {16, 7, 9, 16, 8, 13};  // invented
  static const int one_bit_flags_ = 0x00f;
  static const int two_bit_flags_ = 0xf00;
  static const int one_bit_2_rounds_flags_ = 0x0f0;

 protected:
  ConditionWordPtr K[2];
  ConditionWordPtr M[SIPHASH_MAX_MSG_BLOCKS];
  ConditionWordPtr V[(3 + 4 * SIPHASH_MAX_MSG_BLOCKS + 8)][4];
  ConditionWordPtr A[(3 + 4 * SIPHASH_MAX_MSG_BLOCKS + 8)][4];
  ConditionWordPtr kNumOutputs;

  ConditionWordPtr K2[2];
  ConditionWordPtr M2[SIPHASH_MAX_MSG_BLOCKS];
  ConditionWordPtr V2[(3 + 4 * SIPHASH_MAX_MSG_BLOCKS + 8)][4];
  ConditionWordPtr A2[(3 + 4 * SIPHASH_MAX_MSG_BLOCKS + 8)][4];
  ConditionWordPtr OUT2;
  int num_rounds_;
  int message_blocks_;
  int bitslice_flags_;
  int linear_flags_;
  int rot_[6];
  float best_prob_;

  void SipRound(int round_begin);
  void SipRoundXors(int round_begin);
  void SipRoundTwoBitXors(int round_begin);
  void SipRoundAdds(int round_begin, ConditionWordPtr *c0,
                    ConditionWordPtr *c2);
  void SipRoundCombinedAdds(int round_begin);
  void SipRoundCombinedAddsGraph(int round_begin);
  void SipRoundTwoBitAdds(int round_begin);
  void SipRoundTwoBitCombinedAdds(int round_begin);

  virtual bool Callback(std::string function, Characteristic &characteristic,
                        std::mt19937 &rng, Logfile &logfile);

  void TwoSipRounds(int current_round, bool no_more_round,
                    bool transition_c_to_d, int message_pos);
};

#endif  // SIPHASH_H_

#ifndef SKEIN_H_
#define SKEIN_H_

#include "crypto.h"

#define SKEIN_256_ROUNDS 72
#define WORD_SIZE 4

class Skein : public Crypto {
 public:
  static const uint32_t Rdj[8][2];
  static void AddToOptions(cxxopts::Options& options);
  Skein(cxxopts::Options& options);

 protected:
  int num_rounds_;
  ConditionWordPtr e[SKEIN_256_ROUNDS + 2][WORD_SIZE];
  ConditionWordPtr v[SKEIN_256_ROUNDS + 2][WORD_SIZE];
  ConditionWordPtr tweak[3];
  ConditionWordPtr key[5];
  ConditionWordPtr roundkeys[19][WORD_SIZE];

  ConditionWordPtr e2[SKEIN_256_ROUNDS + 2][WORD_SIZE];
  ConditionWordPtr v2[SKEIN_256_ROUNDS + 2][WORD_SIZE];
  ConditionWordPtr tweak2[3];
  ConditionWordPtr key2[5];
  ConditionWordPtr roundkeys2[19][WORD_SIZE];
};

#endif  // SKEIN_H_

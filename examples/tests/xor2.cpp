#include "tests/xor2.h"

#include "bitslice_step.h"
#include "functions.h"
#include "linear_step.h"

void Xor2::AddToOptions(cxxopts::Options& options) {}

Xor2::Xor2(cxxopts::Options& options) : Crypto(options["word-size"].as<int>()) {
  ConditionWordPtr A = AddConditionWord("A", 0, 0, 0);
  ConditionWordPtr B = AddConditionWord("B", 0, 1, 0);
  ConditionWordPtr C = AddConditionWord("C", 0, 2, 0);
  Add(new BitsliceStep<XOR<2>>(word_size_, A, B, C));
  Add(new LinearStep<XOR<2>>(word_size_, A, B, C));
}

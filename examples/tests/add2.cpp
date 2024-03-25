#include "tests/add2.h"

#include "carry_step.h"
#include "functions.h"

void Add2::AddToOptions(cxxopts::Options& options) {}

Add2::Add2(cxxopts::Options& options) : Crypto(options["word-size"].as<int>()) {
  ConditionWordPtr A = AddConditionWord("A", 0, 0, 0);
  ConditionWordPtr B = AddConditionWord("B", 0, 1, 0);
  ConditionWordPtr C = AddConditionWord("C", 0, 2, 0);
  Add(new CarryStep<ADD<2>>(word_size_, A, B, C));
}

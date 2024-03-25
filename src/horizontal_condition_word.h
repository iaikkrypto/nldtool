#ifndef HORIZONTAL_CONDITION_WORD_H_
#define HORIZONTAL_CONDITION_WORD_H_

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "condition_word.h"

class Step;
class ConditionProxy;
class ConditionWord;
typedef std::shared_ptr<ConditionProxy> ConditionProxyPtr;
//! ConditionWordPtr is just a shared_ptr to  \see ConditionWord
typedef std::shared_ptr<ConditionWord> ConditionWordPtr;

/*!
 * \brief Extension of the normal \see ConditionWord, taking into account
 * twobit-conditions
 */
class HorizontalConditionWord : public ConditionWord {
 public:
  HorizontalConditionWord(int word_index, int storage_index, std::string name,
                          int step_number, int word_type, int word_size,
                          int num_bits);
  HorizontalConditionWord(const HorizontalConditionWord& sw);
  HorizontalConditionWord(uint64_t constant, int word_size = 32,
                          std::string name = "");
  virtual ConditionWordPtr Rotr(int r);
  virtual ConditionWordPtr Rotl(int r);
  virtual ConditionWordPtr Shr(int s);
  virtual ConditionWordPtr Shl(int s);
  virtual ~HorizontalConditionWord(){};
};

#endif  // HORIZONTAL_CONDITION_WORD_H_

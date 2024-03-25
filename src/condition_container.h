#ifndef CONDITION_CONTAINER_H_
#define CONDITION_CONTAINER_H_

#include <cassert>
#include <cmath>
#include <functional>
#include <vector>

#include "bitpos.h"
#include "utils.h"

/*!
 * \brief A container class to hold \see Condition objects and easily get/set
 * them via \see Bitpos.
 */
template <class T>
class ConditionContainer {
 public:
  ConditionContainer(int num_words, int word_size)
      : num_words_(num_words),
        word_size_(word_size),
        log_of_word_size_(log2(word_size_ - 1) + 1),
        conditions_(new T[num_words_ << log_of_word_size_]) {
    assert(word_size_ <= (1 << log_of_word_size_));
  }

  ConditionContainer(const ConditionContainer& c)
      : num_words_(c.num_words_),
        word_size_(c.word_size_),
        log_of_word_size_(c.log_of_word_size_),
        conditions_(new T[num_words_ << log_of_word_size_]) {
    memcpy(conditions_, c.conditions_,
           sizeof(T) * (num_words_ << log_of_word_size_));
  }

  virtual ~ConditionContainer() { delete[] conditions_; }

  ConditionContainer& operator=(const ConditionContainer& c) {
    assert(num_words_ == c.num_words_);
    assert(word_size_ == c.word_size_);
    assert(log_of_word_size_ == c.log_of_word_size_);
    assert(conditions_ != 0);
    memcpy(conditions_, c.conditions_,
           sizeof(T) * (num_words_ << log_of_word_size_));
    return *this;
  }

  virtual void SetCondition(const Bitpos& pos, const T& condition) {
    const int index = pos.GetWord() << log_of_word_size_ | pos.GetBit();
    assert(0 <= pos.GetWord() && pos.GetWord() < num_words_);
    assert(0 <= pos.GetBit() && pos.GetBit() < word_size_);
    assert(0 <= index && index < (num_words_ << log_of_word_size_));
    conditions_[index] = condition;
  }

  virtual T GetCondition(const Bitpos& pos) const {
    const int index = pos.GetWord() << log_of_word_size_ | pos.GetBit();
    assert(0 <= pos.GetWord() && pos.GetWord() < num_words_);
    assert(0 <= pos.GetBit() && pos.GetBit() < word_size_);
    assert(0 <= index && index < (num_words_ << log_of_word_size_));
    return conditions_[index];
  }

  virtual uint64_t GetConditionMask(int word, const T& condition) const {
    assert(0 <= word && word < num_words_);
    const int word_index = word << log_of_word_size_;
    uint64_t mask = 0;
    for (int i = 0; i < word_size_; ++i) {
      const bool b = conditions_[word_index + i] == condition;
      mask |= ((uint64_t)b) << i;
    }
    return mask;
  }

  virtual uint64_t GetConditionMask(int word,
                                    const std::function<bool(T)>& f) const {
    assert(0 <= word && word < num_words_);
    const int word_index = word << log_of_word_size_;
    uint64_t mask = 0;
    for (int i = 0; i < word_size_; ++i) {
      const bool b = f(conditions_[word_index + i]);
      mask |= ((uint64_t)b) << i;
    }
    return mask;
  }

 private:
  int num_words_;
  int word_size_;
  int log_of_word_size_;
  T* conditions_;
};

#endif  // CONDITION_CONTAINER_H_

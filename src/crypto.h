#ifndef CRYPTO_H_
#define CRYPTO_H_

#include <cstdint>
#include <map>
#include <regex>
#include <vector>

#include "bitmask.h"
#include "bitpos.h"
#include "condition_word.h"
#include "cxxopts.hpp"
#include "logfile.h"
#include "step.h"
#include "text_io_format.h"

class Characteristic;

/*!
 * \brief Base class for all crypto functions.
 *
 * All custom crypto functions have to inherit from this class. This class
 * contains all steps added to the crypto as well as all Words.
 */
class Crypto {
 public:
  enum WordTypes { MAINWORD, SUBWORD, CARRYWORD, CONSTANTWORD, HORIZONTALWORD };

  enum ProbabilityCalculation { BITSLICE, CYCLICGRAPH };

  Crypto(int word_size);
  virtual ~Crypto();
  Crypto(const Crypto&) = delete;
  Crypto& operator=(const Crypto&) = delete;
  Crypto(Crypto&&) = delete;
  Crypto& operator=(Crypto&&) = delete;
  virtual bool Callback(std::string function, Characteristic& characteristic,
                        std::mt19937& rng, Logfile& logfile);

  Step* Add(Step* s, Step::Priority priority = {1, 0});

  ConditionWordPtr AddConditionWord(std::string name, int step_number, int row,
                                    int col, int type = MAINWORD,
                                    int num_bits = 1);
  ConditionWordPtr AddReferenceToConditionWord(
      ConditionWordPtr sw, std::string name, int step_number, int row, int col,
      int type, int num_bits = 1, int start_bit = 0);

  const Step& GetStep(int s) const {
    assert(0 <= s && s < steps_.size());
    return *(steps_[s]);
  }

  int GetNumSteps() const { return steps_.size(); }

  int GetNumWords() const { return words_.size(); }

  int GetNumWords(int bits) const {
    assert(0 <= bits && bits < 4);
    return num_words_[bits];
  }

  ConditionWordPtr GetConditionWord(int index) const {
    assert(0 <= index && index < words_.size());
    return words_[index];
  }

  ConditionProxyPtr GetConditionProxy(Bitpos mask_pos) const {
    return GetConditionWord(mask_pos.GetWord())
        ->GetConditionProxy(mask_pos.GetBit());
  }

  int GetWordSize() const { return word_size_; }

  uint64_t GetWordMask() const { return word_mask_; }

  TextIOFormat* GetTextIOFormat() const { return text_io_format_; }

  Bitmask GetWordMaskMain() const;
  Bitmask GetConditionWordMaskRegex(const std::regex& name_regex) const;
  Bitmask GetConditionWordIntervalMaskRegex(const std::regex& name_regex,
                                            int start_step, int end_step) const;

  int GetConditionWordIndex(const std::string& name, int step) const;

 protected:
  int word_size_;
  std::vector<Step*> steps_;

 private:
  void SetupOtherWords();

  std::vector<ConditionWordPtr> words_;
  std::vector<ConditionWordPtr> other_words_;
  int num_words_[4];

  uint64_t word_mask_;
  TextIOFormat* text_io_format_;

  typedef std::pair<std::string, int> WordHandle;
  std::map<WordHandle, int> word_handle_to_index_;
};

#endif  // CRYPTO_H_

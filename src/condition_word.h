#ifndef CONDITION_WORD_H_
#define CONDITION_WORD_H_

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class Step;
class ConditionProxy;
class ConditionWord;
typedef std::shared_ptr<ConditionProxy> ConditionProxyPtr;
//! ConditionWordPtr is just a shared_ptr to  \see ConditionWord
typedef std::shared_ptr<ConditionWord> ConditionWordPtr;

/*!
 * \brief A class describing a word of a certain size and the conditions that
 * are imposed on it.
 *
 * This class provides the functionality to create named words of a certain
 * size. These words are then used in steps as the in- and output for functions
 * like XOR. There are also methods to access a rotated version of the
 * ConditionWordPtr, to reduce the number of steps needed.
 */
class ConditionWord {
 public:
  ConditionWord();
  ConditionWord(uint64_t constant, int word_size = 32, std::string name = "");
  ConditionWord(int condition_mask_index, int container_index, std::string name,
                int step_number, int word_type, int word_size, int num_bits);
  ConditionWord(int condition_mask_index, std::string name, int step_number,
                int word_type, int word_size, int num_bits);
  ConditionWord(int condition_mask_index_, int container_index,
                std::string name, int step_number, int word_type, int num_bits);
  ConditionWord(const ConditionWord& sw);
  ConditionWord(const ConditionWord& sw, std::string name, int step_number,
                int word_type, int num_bits);
  ConditionWord& operator=(const ConditionWord& sw);
  void SetName(const std::string& name);
  std::string GetName() const;
  void SetStepNumber(const int step_number);
  int GetStepNumber() const;
  int GetWordType() const;
  bool IsMainWord() const;
  bool IsSubWord() const;
  bool IsCarryWord() const;
  bool IsConstantWord() const;
  bool IsHorizontalWord() const;
  int GetWordSize() const;
  ConditionProxyPtr GetConditionProxy(int bit) const;
  void SetConditionProxy(int bit, ConditionProxyPtr cp);
  int GetConditionMaskIndex() const;
  bool ContainsOnlyContainerConditionProxies() const;
  int GetContainerIndex() const;
  int GetNumBits() const;
  friend std::ostream& operator<<(std::ostream& os, const ConditionWord& s);
  friend std::ostream& operator<<(std::ostream& os, const ConditionWord* s);
  void Print(std::ostream& os) const;
  virtual ConditionWordPtr Rotr(int r);
  virtual ConditionWordPtr Rotl(int r);
  virtual ConditionWordPtr Shr(int s);
  virtual ConditionWordPtr Shl(int s);
  ConditionWordPtr SetZero(int bit, int num, int start);
  void SetStepToComputeProbability(Step* step);
  Step* GetStepToComputeProbability();
  virtual ~ConditionWord(){};

 protected:
  int container_index_;
  int condition_mask_index_;
  int num_bits_;
  std::vector<ConditionProxyPtr> condition_proxies_;
  std::string name_;
  int step_number_;
  int word_type_;
  Step* step_to_compute_probability_;
  bool only_container_condition_proxies_;
};

#endif  // CONDITION_WORD_H_

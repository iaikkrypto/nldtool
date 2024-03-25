#ifndef CHARACTERISTIC_H_
#define CHARACTERISTIC_H_

#include <cstdint>
#include <string>
#include <vector>

#include "bitmask.h"
#include "bitpos.h"
#include "condition.h"
#include "condition_container.h"
#include "crypto.h"
#include "logfile.h"
#include "step_data.h"
#include "step_update.h"
#include "twobit_container.h"

/*!
 * \brief Representation of the current state of a differential characteristic
 * of a specific crypto function.
 *
 * The Characteristic class contains all data concerning the current state of
 * the differential characteristic. It contains references to \see Crypto, \see
 * Condition and \see StepData, and contains the main logic for the propagation
 * of differences through the different functions and steps. It also provides
 * helper functions that enable easy output of the current state.
 */
class Characteristic {
 public:
  Characteristic(CryptoPtr crypto);
  Characteristic(const Characteristic& s);
  Characteristic& operator=(const Characteristic& s);
  // move operators
  Characteristic(Characteristic&& s);
  Characteristic& operator=(Characteristic&& s);
  virtual ~Characteristic();
  void ShallowCopy(const Characteristic& s);
  void Undo();
  void UndoAndRestore();

  BitCondition GetContainerCondition1(Bitpos container_pos) const {
    return bit_conditions_.GetCondition(container_pos);
  }

  Condition<2> GetContainerCondition2(Bitpos container_pos) const {
    return bit_conditions2_.GetCondition(container_pos);
  }

  Condition<3> GetContainerCondition3(Bitpos container_pos) const {
    return bit_conditions3_.GetCondition(container_pos);
  }

  bool SetContainerCondition1(Bitpos container_pos, BitCondition value) {
    bit_conditions_.SetCondition(container_pos, value);
    return value != BitCondition("#");
  }

  bool SetContainerCondition2(Bitpos container_pos, Condition<2> value) {
    bit_conditions2_.SetCondition(container_pos, value);
    return value != Condition<2>(0);
  }

  bool SetContainerCondition3(Bitpos container_pos, Condition<3> value) {
    bit_conditions3_.SetCondition(container_pos, value);
    return value != Condition<3>(0);
  }

  BitCondition GetBitCondition(Bitpos condition_mask_pos) const;

  bool SetBitCondition(Bitpos condition_mask_pos, BitCondition value);

  bool PurgeBitCondition(Bitpos condition_mask_pos, BitCondition value) {
    return SetBitCondition(condition_mask_pos,
                           value.Filter(GetBitCondition(condition_mask_pos)));
  }

  CryptoPtr GetCrypto() const;
  StepData& GetStepData(int step);
  const StepData& GetStepData(int step) const;
  void SetBitConditionWord(int word_index, BitCondition value,
                           uint64_t bit_mask = ~0ull);
  uint64_t GetConditionWordMask(const std::function<bool(BitCondition)>& f,
                                int i) const;
  Bitmask GetBitConditionMask(Bitmask mask, BitCondition value) const;
  Bitmask GetBitConditionMask(BitCondition value) const;
  Bitmask GetConditionMask(Bitmask mask,
                           const std::function<bool(BitCondition)>& f) const;
  Bitmask GetConditionMask(const std::function<bool(BitCondition)>& f) const;
  std::vector<Bitpos> GetBitConditionList(int word, BitCondition value) const;

  void SetBit(const std::string& name, int step, int bit, bool a, bool b);
  void SetWord(const std::string& name, int step, uint64_t a, uint64_t b);

  bool GetWord(const std::string& name, int step, uint64_t& a, uint64_t& b);

  bool Update(bool backtrack = false, int32_t priority = 10000);
  bool UpdateAll();
  void Touch(const ConditionProxy& cp);
  bool Complete(const std::function<bool(BitCondition)>& f);
  bool Complete(const std::function<bool(BitCondition)>& f,
                const Bitmask& mask);

  // checkLevel 1: update
  // checkLevel 2: complete on all 2-bit conditions
  // checkLevel 3: complete on all -/x conditions
  int CheckCharacteristic(Logfile& logfile, int check_level = 2,
                          bool print_characteristic = false);

  Bitmask& GetTwobitUpdateMask();
  TwobitContainer& GetTwobitConditions();
  const TwobitContainer& GetConstTwobitConditions() const;
  void GenerateTwobitConditions();

  float GetProbability(int word_index) const;
  float GetProbabilitySum(int step_number) const;
  float GetProbabilitySum() const;

  bool ReadCharacteristic(std::istream& fs, bool only_main_steps);
  void WriteCharacteristic(std::string file);
  void WriteCharacteristic(std::ostream& fs);
  void WriteCharacteristic(Logfile& logfile);

 private:
  CryptoPtr crypto_;

  ConditionContainer<BitCondition> bit_conditions_;
  ConditionContainer<Condition<2>> bit_conditions2_;
  ConditionContainer<Condition<3>> bit_conditions3_;
  std::vector<StepData*> step_data_;

  TwobitContainer twobit_container_;

  std::set<StepUpdate> step_update_list_;
  std::set<Bitpos, std::less<Bitpos>> condition_update_list_;
  Bitmask twobit_bitslice_update_mask_;
};

#endif  // CHARACTERISTIC_H_

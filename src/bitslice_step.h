#ifndef BITSLICE_STEP_H_
#define BITSLICE_STEP_H_

#include <cassert>
#include <cmath>
#include <cstdint>
#include <set>
#include <vector>

#include "bitpos.h"
#include "bitslice.h"
#include "bitslice_data.h"
#include "bitslice_step_data.h"
#include "characteristic.h"
#include "condition_word.h"
#include "crypto.h"
#include "step.h"
#include "utils.h"

//#define DEBUG_STEP

struct Mapping {
  const int* mapping_function_;
  int mapping_function_pos_;
};

/*!
 * \brief A description of a bitslice step.
 *
 * The bitsliced function of a step propagates the conditions by computing all
 * possible inputs for the imposed conditions, then executing the function and
 * combining all outputs back into the respective conditions.
 */
template <class F>
class BitsliceStep : public Step {
 public:
  template <class... Types>
  BitsliceStep(Types... rest) : Step(rest...) {}

  BitsliceStep(int word_size, const std::vector<ConditionWordPtr>& params)
      : Step(word_size, params) {}

  virtual ~BitsliceStep() {}

  virtual void Init(int step_index, Priority priority) {
    step_index_ = step_index;
    priority_ = priority.mul * BitsliceData<F>::LOOPSIZE + priority.add;
    // std::cout << GetName() << " priority: " << priority_ << std::endl;
    for (int bit = 0; bit < word_size_; bit++) {
      for (int i = 0; i < GetNumParams(); i++) {
        StepUpdate pos;
        pos.priority = priority_;
        pos.step = step_index_;
        pos.bit = bit;
        GetConditionProxy(i, bit)->AddStepToUpdate(pos);
      }
    }
    Bitslice<F>::InitCaches();
    InitMapping();
  }

  virtual void InitMapping() {
    int num_bits = GetConditionProxy(F::kPrevState, 0)->GetNumBits();
    for (int bit_index = 0; bit_index < word_size_; bit_index++) {
      uint8_t bits_const = 0;
      for (int bits = 0; bits < num_bits; ++bits)
        bits_const |=
            (GetConditionProxy(F::kPrevState, bit_index)->IsPosKonstant(bits)
             << bits);

#ifdef DEBUG_STEP
      std::cout << bit_index << ": ";
      for (int bits = 0; bits < num_bits; ++bits)
        std::cout << bits_const[bits] << " ";
      std::cout << std::endl;
#endif
      if (bits_const) switch (num_bits) {
          case 1:
            mapping_.push_back(
                {merge_states1_, (bit_index - 1 + word_size_) % word_size_});
            break;
          case 2:
            mapping_.push_back({merge_states2_[bits_const - 1],
                                (bit_index - 1 + word_size_) % word_size_});
            break;
          case 3:
            mapping_.push_back({merge_states3_[bits_const - 1],
                                (bit_index - 1 + word_size_) % word_size_});
            break;
          default:
            assert(!"something wrong here");
            break;
        }
    }
  }

  virtual std::string GetName() const { return std::string(F::kName); }

  virtual BitsliceStepData* CreateStepData() const {
    return new BitsliceStepData();
  }

  virtual bool Update(Characteristic& characteristic,
                      std::set<Bitpos>& condition_pos, int step) const {
    return true;
  }

  virtual bool Update(Characteristic& characteristic, int pos,
                      bool backtrack = false) const {
    bool result = true;
    BitsliceData<F> input, output;
    for (int i = 0; i < F::kNumInputs + F::kNumOutputs; i++)
      input.SetCondition(
          i, GetConditionProxy(i, pos)->GetCondition(characteristic));
    output = Bitslice<F>::Compute(input);
    for (int i = 0; i < F::kNumInputs + F::kNumOutputs; i++)
      result &= GetConditionProxy(i, pos)->SetCondition(characteristic,
                                                        output.GetCondition(i));
#ifdef DEBUG_STEP
    std::cout << F::kName << Bitpos(step_index_, pos) << ": ";
    std::cout << input << " -> " << output;
    std::cout << std::endl;
#endif
    return result;
  }

  virtual std::vector<TwobitCondition> UpdateTwobitCondition(
      Characteristic& characteristic, Bitpos pos) const {
    BitsliceData<F> input;
    PropagateTwobitOutput output;
    for (int i = 0; i < F::kNumInputs + F::kNumOutputs; i++)
      input.SetCondition(
          i, GetConditionProxy(i, pos.GetBit())->GetCondition(characteristic));
    output = Bitslice<F>::ComputeTwobit(input);
    std::vector<TwobitCondition> twobit;
    twobit.reserve(output.twobit_list_.size());
    for (int i = 0; i < output.twobit_list_.size(); i++) {
      ConditionProxyPtr cpa =
          GetConditionProxy(output.twobit_list_[i].a, pos.GetBit());
      ConditionProxyPtr cpb =
          GetConditionProxy(output.twobit_list_[i].b, pos.GetBit());
      if (nldtool::HammingWeight(cpa->GetCondition(characteristic)) == 1 ||
          nldtool::HammingWeight(cpb->GetCondition(characteristic)) == 1)
        continue;
      //      std::shared_ptr<MaskConditionProxy>
      //      vcpa(std::dynamic_pointer_cast<MaskConditionProxy>(cpa));
      //      std::shared_ptr<MaskConditionProxy>
      //      vcpb(std::dynamic_pointer_cast<MaskConditionProxy>(cpb));
      if (!cpa->IsMaskConditionProxy() || !cpb->IsMaskConditionProxy())
        continue;
      Bitpos cond_pos_a = cpa->GetConditionMaskPos();
      Bitpos cond_pos_b = cpb->GetConditionMaskPos();
      if (cond_pos_a < cond_pos_b)
        twobit.push_back(
            TwobitCondition(cond_pos_a, cond_pos_b, output.twobit_list_[i].c));
      else
        twobit.push_back(
            TwobitCondition(cond_pos_b, cond_pos_a, output.twobit_list_[i].c));
    }
#ifdef DEBUG_STEP
    std::cout << F::kName << bitslice_pos << ": " << input << " -> " << output
              << " -> ";
    for (int i = 0; i < twobit.size(); i++) std::cout << twobit[i] << " ";
    std::cout << std::endl;
#endif
    return twobit;
  }

  const std::multimap<Bitpos, int>& GetBitslicesToUpdate() {
    return bitslices_to_update_;
  }

  virtual float GetProbability(const Characteristic& characteristic) const {
    float probability = 0.0;
    switch (probability_method_) {
      case Crypto::BITSLICE:  // probability for every single bitslice
        for (int pos = 0; pos < word_size_; ++pos) {
          BitsliceData<F> input;
          for (int i = 0; i < F::kNumInputs + F::kNumOutputs; i++)
            input.SetCondition(
                i, GetConditionProxy(i, pos)->GetCondition(characteristic));
          probability += Bitslice<F>::ComputeProbability(input).probability_;
        }
        break;

      case Crypto::CYCLICGRAPH:  // probability ...?
        std::vector<ProbabilityOutputMatrix> matrices;
        matrices.reserve(word_size_);
        for (int pos = 0; pos < word_size_; ++pos) {
          BitsliceData<F> input;
          for (int i = 0; i < F::kNumInputs + F::kNumOutputs; ++i)
            input.SetCondition(
                i, GetConditionProxy(i, pos)->GetCondition(characteristic));
          matrices.push_back(Bitslice<F>::ComputeProbabilityMatrix(input));
        }
        // merge states
        for (std::vector<Mapping>::const_iterator it = mapping_.begin();
             it != mapping_.end(); ++it) {
          const int* mapping_function = it->mapping_function_;
          int mapping_pos = it->mapping_function_pos_;
          for (int column = 0; column < F::kStateSize; ++column)
            for (int row = 0; row < F::kStateSize; ++row)
              if (mapping_function[row] != row) {
                matrices[mapping_pos].Set(
                    mapping_function[row], column,
                    matrices[mapping_pos].Get(row, column) +
                        matrices[mapping_pos].Get(mapping_function[row],
                                                  column));
                matrices[mapping_pos].Set(row, column, 0);
              }
        }
        for (int pos = word_size_ - 2; pos >= 0; --pos)
          matrices[word_size_ - 1] = matrices[word_size_ - 1] * matrices[pos];
        for (int i = 0; i < F::kStateSize; i++)
          probability += matrices[word_size_ - 1].Get(i, i);
        probability = (float)(log(probability) / log(2.0));
        break;
    }
    return probability;
  }

  const int merge_states1_[4] = {0, 0, 0, 0};
  const int merge_states2_[3][16] = {
      {0, 0, 0, 0, 4, 5, 4, 5, 8, 4, 5, 4, 5, 8, 8, 8},
      {0, 1, 2, 3, 0, 0, 2, 1, 0, 1, 2, 3, 3, 1, 2, 3},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};
  const int merge_states3_[7][64] = {
      {0,  0,  0,  0,  4,  5,  4,  5,  8,  4,  5,  4,  5,  8,  8,  8,
       16, 17, 16, 17, 20, 21, 20, 21, 24, 16, 17, 16, 17, 20, 21, 20,
       21, 24, 24, 24, 36, 37, 36, 37, 40, 41, 40, 41, 44, 45, 44, 45,
       48, 36, 37, 36, 37, 40, 41, 40, 41, 44, 45, 44, 45, 48, 48, 48},
      {0,  1,  2,  3,  0,  0,  2,  1,  0,  1,  2,  3,  3,  1,  2,  3,
       16, 17, 18, 19, 16, 17, 18, 19, 24, 25, 26, 27, 28, 25, 26, 27,
       28, 33, 34, 35, 16, 17, 18, 19, 16, 17, 18, 19, 24, 24, 34, 33,
       24, 25, 26, 27, 28, 25, 26, 27, 28, 33, 34, 35, 35, 33, 34, 35},
      {0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
       16, 17, 16, 17, 16, 17, 16, 17, 24, 16, 17, 16, 17, 16, 17, 16,
       17, 24, 24, 24, 16, 17, 16, 17, 16, 17, 16, 17, 24, 24, 24, 24,
       24, 16, 17, 16, 17, 16, 17, 16, 17, 24, 24, 24, 24, 24, 24, 24},
      {0,  1, 2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
       0,  0, 2,  1,  5,  4,  10, 9,  0,  1, 2,  3,  3,  7,  6,  12,
       11, 1, 2,  3,  4,  5,  6,  7,  8,  8, 14, 13, 4,  5,  6,  7,
       8,  9, 10, 11, 12, 13, 14, 15, 15, 9, 10, 11, 12, 13, 14, 15},
      {0},
      {0, 1, 2, 3, 0, 0, 2, 1, 0, 1, 2, 3, 3, 1, 2, 3, 0, 0, 2, 1, 0, 0,
       2, 1, 0, 1, 2, 3, 3, 1, 2, 3, 3, 1, 2, 3, 0, 0, 2, 1, 0, 0, 2, 1,
       0, 0, 2, 1, 0, 1, 2, 3, 3, 1, 2, 3, 3, 1, 2, 3, 3, 1, 2, 3},
      {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};

 protected:
  std::multimap<Bitpos, int> bitslices_to_update_;
  std::vector<Mapping> mapping_;
};

#endif  // BITSLICE_STEP_H_

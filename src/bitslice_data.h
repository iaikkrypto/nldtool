#ifndef BITSLICE_DATA_H_
#define BITSLICE_DATA_H_

#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <string>

#include "bitslice_pair.h"
#include "index.h"
#include "pair.h"
#include "utils.h"
#include "word_container.h"

//#define DIFFTWOBIT
//#define DEBUG_LOOP 1
//#define DEBUG_ALLPOSS 1
//#define DEBUG_LOOP (NUMWORDS==3)
//#define DEBUG_ALLPOSS (NUMWORDS==3)
//#define DEBUG_TWOBIT

/*!
 * \brief Data representation of the condition in- and outputs to bitslice
 * functions.
 */
template <class F>
class BitsliceData {
 private:
  static constexpr int ConditionIndex(int i) {
    return i == 0 ? 0
                  : ConditionIndex(i - 1) + (1ull << (2 * F::Bitsize(i - 1)));
  }

  template <int N, int... Vals>
  static constexpr
      typename std::enable_if<N == sizeof...(Vals), std::array<int, N>>::type
      GenerateConditionIndex() {
    return std::array<int, N>{{Vals...}};
  }

  template <int N, int... Vals>
  static constexpr
      typename std::enable_if<N != sizeof...(Vals), std::array<int, N>>::type
      GenerateConditionIndex() {
    return GenerateConditionIndex<N, Vals...,
                                  ConditionIndex(sizeof...(Vals))>();
  }

 public:
  static constexpr std::array<int, F::kNumInputs + F::kNumOutputs + 1> CINDEX =
      GenerateConditionIndex<F::kNumInputs + F::kNumOutputs + 1>();
  static constexpr int NUMWORDS =
      (CINDEX[F::kNumInputs + F::kNumOutputs] - 1) / 64 + 1;
  static constexpr int NUMBITS = CINDEX[F::kNumInputs + F::kNumOutputs];
  static constexpr int LOOPSIZE = CINDEX[F::kNumInputs];

  WordContainer<NUMWORDS> conditions_;

  BitsliceData() : conditions_() {}
  BitsliceData(const BitsliceData& b) : conditions_(b.conditions_) {}

  static std::string GetName() { return F::kName; }

  static BitsliceData<F> GetEmptyKey() { return BitsliceData<F>(); }

  static BitsliceData<F> GetDeletedKey() {
    BitsliceData<F> tmp;
    tmp.conditions_.Set(0, 1, true);
    return tmp;
  }

  BitsliceData<F>& operator=(BitsliceData<F> b) {
    conditions_ = b.conditions_;
    return *this;
  }

  operator uint64_t() const {
    uint64_t result = 0;
    for (int i = 0; i < F::kNumInputs + F::kNumOutputs; ++i)
      result |= GetCondition(i) << (i * 4);
    return result;
  }

  uint64_t operator()(const BitsliceData<F>& i) const {
    return i.conditions_.Hash();
  }

  bool operator()(const BitsliceData<F>& i1, const BitsliceData<F>& i2) const {
    return i1.conditions_.Equals(i2.conditions_);
  }

  friend std::ostream& operator<<(std::ostream& os,
                                  const BitsliceData<F>& cbs) {
    os << std::hex;
    for (int i = F::kNumInputs + F::kNumOutputs - 1; i > 0; --i)
      os << cbs.GetCondition(i) << ".";
    os << cbs.GetCondition(0);
    os << std::dec;
    return os;
  }

  bool Compare(const BitsliceData<F>& cbs) {
    return conditions_.Equals(cbs.conditions_);
  }

  uint64_t GetCondition(int i) const {
    return conditions_.Get(CINDEX[i], CINDEX[i + 1] - 1);
  }

  void SetCondition(int i, const uint64_t& cond) {
    conditions_.Set(CINDEX[i], CINDEX[i + 1] - 1, cond);
  }

  void SwapCondition(int a, int b) {
    const uint64_t aa = GetCondition(a);
    const uint64_t bb = GetCondition(b);
    SetCondition(a, bb);
    SetCondition(b, aa);
  }

  void SortInput(uint8_t perm[]) {
    for (int i = 0; i < F::kNumInputs + F::kNumOutputs; i++)
      for (int j = i + 1; j < F::kNumInputs + F::kNumOutputs; j++)
        if (F::Symmetry(i) == F::Symmetry(j) &&
            GetCondition(i) < GetCondition(j)) {
          SwapCondition(i, j);
          std::swap(perm[i], perm[j]);
        }
  }

  void ResortOutput(uint8_t perm[], int len) {
    BitsliceData<F> tmp = *this;
    for (int i = 0; i < len; i++) {
      SetCondition(perm[i], tmp.GetCondition(i));
    }
  }

  bool SetupPairs(Pair pairs[64][F::kNumInputs],
                  Index<F::kNumInputs>& indices) {
    for (int j = F::kNumInputs - 1; j >= 0; --j) {
      assert(1 <= F::Bitsize(j) && F::Bitsize(j) <= 3);
      const int num = 1 << F::Bitsize(j);
      const uint64_t cond = GetCondition(j);
      indices.SetSize(j, 0);
      for (int a = 0; a < num; ++a) {
        for (int b = 0; b < num; ++b) {
          const uint64_t c = nldtool::GetBit(cond, b * num + a);
          pairs[indices.GetSize(j)][j] = Pair{uint8_t(a), uint8_t(b)};
          indices.AddToSize(j, c);
        }
      }
      DebugPairs(pairs, indices, j);
      if (indices.GetSize(j) == 0) return false;
    }
    DebugInput(indices);
    return true;
  }

  BitslicePair<F> GetPair(const Pair pairs[64][F::kNumInputs],
                          Index<F::kNumInputs> indices) const {
    DebugIndices(indices);
    BitslicePair<F> val;
    for (int j = 0; j < F::kNumInputs; ++j) {
      val[j] = pairs[indices[j]][j];
      DebugUpdatePairs(pairs, indices, val, j);
    }
    return val;
  }

  bool IsPair(BitslicePair<F> val) const {
    bool ispair = true;
    for (int j = 0; j < F::kNumOutputs; ++j) {
      const int i = F::kNumInputs + j;
      const uint64_t condition = GetCondition(i);
      const uint64_t pair = val.GetPair(i);
      ispair &= (condition & pair) != 0;
    }
    return ispair;
  }

#ifdef DEBUG_ALLPOSS
  void DebugIndices(Index<F::kNumInputs> indices) const {
    if (!DEBUG_ALLPOSS) return;
    std::cout << "indices: ";
    for (int i = F::kNumInputs - 1; i >= 0; --i) std::cout << indices[i] << " ";
    std::cout << std::endl;
  }

  void DebugUpdatePairs(const Pair pairs[64][F::kNumInputs],
                        Index<F::kNumInputs> indices, BitslicePair<F> val,
                        int j) const {
    if (!DEBUG_ALLPOSS) return;
    std::cout << "  update pairs[" << j << "][" << indices[j] << "]=("
              << pairs[indices[j]][j].second << ","
              << pairs[indices[j]][j].first << ")"
              << " -> " << val << std::endl;
  }

  void DebugPairs(const Pair pairs[64][F::kNumInputs],
                  Index<F::kNumInputs> indices, int j) const {
    if (!DEBUG_ALLPOSS) return;
    std::cout << "  pairs[" << j << "][*]: ";
    for (int a = 0; a < indices.GetSize(j); ++a)
      std::cout << "(" << pairs[a][j].second << "," << pairs[a][j].first
                << ") ";
    std::cout << std::endl;
  }

  void DebugInput(Index<F::kNumInputs> indices) const {
    if (!DEBUG_ALLPOSS) return;
    std::cout << "input=" << (*this) << "    lengths: ";
    for (int i = F::kNumInputs - 1; i >= 0; --i)
      std::cout << indices.GetSize(i) << " ";
    std::cout << std::endl;
  }
#else
  void DebugIndices(Index<F::kNumInputs> indices) const {}
  void DebugUpdatePairs(const Pair pairs[64][F::kNumInputs],
                        Index<F::kNumInputs> indices, BitslicePair<F> val,
                        int j) const {}
  void DebugPairs(const Pair pairs[64][F::kNumInputs],
                  Index<F::kNumInputs> indices, int j) const {}
  void DebugInput(Index<F::kNumInputs> indices) const {}
#endif

  const char* GetBytePtr() const {
    return reinterpret_cast<const char*>(&conditions_);
  }

  int GetByteSize() const { return sizeof(WordContainer<NUMWORDS>); }

  void SetFromBytePtr(const char* data, int size) {
    // assert(size == sizeof(WordContainer<GETWORDS>));
    memcpy(&conditions_, data, size);
  }
};

template <class F>
constexpr std::array<int, F::kNumInputs + F::kNumOutputs + 1>
    BitsliceData<F>::CINDEX;

#endif  // BITSLICE_DATA_H_

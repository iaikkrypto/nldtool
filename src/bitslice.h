#ifndef BITSLICE_H_
#define BITSLICE_H_

#include <cassert>
#include <cstdint>
#include <iostream>

#include "cache.h"
#include "probability.h"
#include "probability_matrix.h"
#include "probability_output.h"
#include "propagate.h"
#include "propagate_twobit.h"
#include "propagate_twobit_output.h"
#include "utils.h"

/*!
 * \brief The bitslice for a given function, with a caching mechanism for
 * faster access to already calculated results.
 *
 * The class has a cache for the function calls. For small functions like \see
 * XOR of two inputs, the cache is static and calculated once at the beginning
 * of the search. For larger functions, the cache is build dynamically during
 * the search. The input conditions of a bitslice function are expanded and then
 * the Loop function calculates all outputs, using the appropriate \see Action
 * to combine the results.
 */
template <class F>
class Bitslice {
 public:
  static void InitCaches() {
    InitStaticCachePropagate();
    InitStaticCacheProbability();
    InitStaticCachePropagateTwobit();
  }

  static void InitStaticCachePropagate() {
    std::string filepath =
        "CacheDump_" + Propagate<F>::GetName() + "_" + F::kName + ".db";
    cache_.SetDumpFilepathAndLoadDump(filepath.c_str());
    if (cache_.IsInitialized()) return;
    BitsliceData<F> input;
    BitsliceData<F> output;
    uint64_t bitsize = BitsliceData<F>::NUMBITS;
    uint64_t size = 1ull << bitsize;
    std::cout << "init static cache of " << F::kName
              << " for Propagate with size " << size << " (" << bitsize
              << ") ... ";
    std::cout.flush();
    cache_.SetSize(size);
    for (int64_t i = 0; i < size; i++) {
      for (int j = 0; j < F::kNumInputs + F::kNumOutputs; j++)
        input.SetCondition(j, (i >> BitsliceData<F>::CINDEX[j]) &
                                  nldtool::Mask(1ull << (2 * F::Bitsize(j))));
      output = Loop<Propagate<F>>(input);
      cache_.Insert(input, output);
    }
    cache_.SetInitialized();
    std::cout << "done" << std::endl;
  }

  static void InitStaticCacheProbability() {
    if (probability_cache_.IsInitialized()) return;
    BitsliceData<F> input;
    ProbabilityOutput output;
    uint64_t bitsize = BitsliceData<F>::NUMBITS;
    uint64_t size = 1ull << bitsize;
    std::cout << "init static cache of " << F::kName
              << " for Probability with size " << size << " (" << bitsize
              << ") ... ";
    std::cout.flush();
    probability_cache_.SetSize(size);
    for (int64_t i = 0; i < size; i++) {
      for (int j = 0; j < F::kNumInputs + F::kNumOutputs; j++)
        input.SetCondition(j, (i >> BitsliceData<F>::CINDEX[j]) &
                                  nldtool::Mask(1ull << (2 * F::Bitsize(j))));
      output = Loop<Probability<F>>(input);
      probability_cache_.Insert(input, output);
    }
    probability_cache_.SetInitialized();
    std::cout << "done" << std::endl;
  }

  static void InitStaticCachePropagateTwobit() {
    std::string filepath =
        "CacheDump_" + PropagateTwobit<F>::GetName() + "_" + F::kName + ".db";
    twobit_cache_.SetDumpFilepathAndLoadDump(filepath.c_str());
    if (twobit_cache_.IsInitialized()) return;
    BitsliceData<F> input;
    PropagateTwobitOutput output;
    uint64_t bitsize = BitsliceData<F>::NUMBITS;
    uint64_t size = 1ull << bitsize;
    std::cout << "init static cache of " << F::kName
              << " for PropagateTwobit with size " << size << " (" << bitsize
              << ") ... ";
    std::cout.flush();
    twobit_cache_.SetSize(size);
    for (int64_t i = 0; i < size; i++) {
      for (int j = 0; j < F::kNumInputs + F::kNumOutputs; j++)
        input.SetCondition(j, (i >> BitsliceData<F>::CINDEX[j]) &
                                  nldtool::Mask(1ull << (2 * F::Bitsize(j))));
      output = Loop<PropagateTwobit<F>>(input);
      twobit_cache_.Insert(input, output);
    }
    twobit_cache_.SetInitialized();
    std::cout << "done" << std::endl;
  }

  static BitsliceData<F> Compute(BitsliceData<F> input) {
    typename CacheBase<BitsliceData<F>, BitsliceData<F>>::Match result;
    BitsliceData<F> output;
    uint8_t perm[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    input.SortInput(perm);
    result = cache_.Find(input);
    if (result.matched) {
      output = result.second;
    } else {
      output = Loop<Propagate<F>>(input);
      cache_.Insert(input, output);
    }
    output.ResortOutput(perm, F::kNumInputs + F::kNumOutputs);
    return output;
  }

  static ProbabilityOutput ComputeProbability(BitsliceData<F> input) {
    typename CacheBase<BitsliceData<F>, ProbabilityOutput>::Match result;
    ProbabilityOutput output;
    uint8_t perm[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    input.SortInput(perm);
    result = probability_cache_.Find(input);
    if (result.matched) {
      output = result.second;
    } else {
      output = Loop<Probability<F>>(input);
      probability_cache_.Insert(input, output);
    }
    return output;
  }

  static ProbabilityOutputMatrix ComputeProbabilityMatrix(
      BitsliceData<F> input) {
    typename CacheBase<BitsliceData<F>, ProbabilityOutputMatrix>::Match result;
    ProbabilityOutputMatrix output;
    uint8_t perm[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    input.SortInput(perm);
    result = probability_matrix_cache_.Find(input);
    if (result.matched) {
      output = result.second;
    } else {
      output = Loop<ProbabilityMatrix<F>>(input);
      probability_matrix_cache_.Insert(input, output);
    }
    return output;
  }

  static PropagateTwobitOutput ComputeTwobit(BitsliceData<F> input) {
    typename CacheBase<BitsliceData<F>, PropagateTwobitOutput>::Match result;
    PropagateTwobitOutput output;
    uint8_t perm[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    input.SortInput(perm);
    result = twobit_cache_.Find(input);
    if (result.matched) {
      output = result.second;
    } else {
      output = Loop<PropagateTwobit<F>>(input);
      twobit_cache_.Insert(input, output);
    }
    output.ResortOutput(perm, F::kNumInputs + F::kNumOutputs);
    return output;
  }

  template <class A>
  static typename A::Output Loop(typename A::Input input) {
    A action;
    action.Initialize(input);

#ifdef DEBUG_LOOP
    if (DEBUG_LOOP) std::cout << "in:  " << input << std::endl;
#endif

    Index<F::kNumInputs> indices = {};
    Pair pairs[64][F::kNumInputs];
    int size = 0;

    if (!input.SetupPairs(pairs, indices)) return action.Finalize(size);

    do {
      BitslicePair<F> pair = input.GetPair(pairs, indices);
      size++;

      F::f(pair.x, pair.x + F::kNumInputs);

      for (int i = 0; i < F::kNumInputs + F::kNumOutputs; ++i)
        if ((pair.x[i] & nldtool::Mask(F::Bitsize(i))) != pair.x[i]) {
          std::cout << F::kName << " " << i << "(" << int(F::Bitsize(i))
                    << "): (" << int(pair.x[i].first) << ","
                    << int(pair.x[i].second) << ")" << std::endl;
          assert(!"value too large for condition");
        }

#ifdef DEBUG_LOOP
      if (DEBUG_LOOP) std::cout << "  pair: " << pair << " -> ";
#endif

      if (input.IsPair(pair)) {
#ifdef DEBUG_LOOP
        if (DEBUG_LOOP) std::cout << "collect";
#endif
        action.Collect(pair);
      }
#ifdef DEBUG_LOOP
      else if (DEBUG_LOOP)
        std::cout << "x";
      if (DEBUG_LOOP) std::cout << std::endl;
#endif
    } while (indices.Increment());

    typename A::Output output = action.Finalize(size);
#ifdef DEBUG_LOOP
    if (DEBUG_LOOP) std::cout << "out: " << output << std::endl;
#endif
    return output;
  }

  static bool loadDump() {
    if (cache_.loadDump() == false) return false;
    return twobit_cache_.loadDump();
  }

  static bool saveDump() {
    if (cache_.saveDump() == false) return false;
    return twobit_cache_.saveDump();
  }

 private:
  static typename Cache<BitsliceData<F>, BitsliceData<F>>::result cache_;
  static typename Cache<BitsliceData<F>, ProbabilityOutput>::result
      probability_cache_;
  static typename Cache<BitsliceData<F>, ProbabilityOutputMatrix>::result
      probability_matrix_cache_;
  static typename Cache<BitsliceData<F>, PropagateTwobitOutput>::result
      twobit_cache_;
};

template <class F>
typename Cache<BitsliceData<F>, BitsliceData<F>>::result Bitslice<F>::cache_;
template <class F>
typename Cache<BitsliceData<F>, ProbabilityOutput>::result
    Bitslice<F>::probability_cache_;
template <class F>
typename Cache<BitsliceData<F>, ProbabilityOutputMatrix>::result
    Bitslice<F>::probability_matrix_cache_;
template <class F>
typename Cache<BitsliceData<F>, PropagateTwobitOutput>::result
    Bitslice<F>::twobit_cache_;

#endif  // BITSLICE_H_

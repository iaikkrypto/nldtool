#ifndef WORD_CONTAINER_H_
#define WORD_CONTAINER_H_

#include <cassert>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <unordered_map>  // needed for std::hash

#include "utils.h"

//#define USE_STD_HASH
#define USE_STD_HASH_CHAIN 64  // 32 or 64
#if USE_STD_HASH_CHAIN == 32
#define CHAIN_VALUE 0x9e3779b9
#elif USE_STD_HASH_CHAIN == 64
#define CHAIN_VALUE 0x9b9773e99e3779b9
#endif

static const uint64_t MOD = 2147483648ull;

/*!
 * \brief A container holding N words, with getters/setters and custom hash
 * function.
 */
template <int N>
class WordContainer {
 private:
  uint64_t words_[N];

 public:
  uint64_t Hash() const {
#ifdef USE_STD_HASH
    // the algorithm to combine two hash values is taken from std::hash
    uint64_t result = 14695981039346656037ULL;
    for (int i = 0; i < N; ++i) {
      result ^= words_[i];
      result *= 1099511628211ULL;
    }
    return result;
#else
#ifdef USE_STD_HASH_CHAIN
    // the algorithm to combine two hash values is taken from boost
    // hash_combine()
    std::hash<uint64_t> hasher;
    uint64_t result = 0;
    for (int i = 0; i < N; ++i)
      result ^= hasher(words_[i]) + CHAIN_VALUE + (result << 6) + (result >> 2);
    return result;
#else
    uint64_t result = 0;
    for (int i = 0; i < N; ++i) result += words_[i];
    return result % MOD;
#endif
#endif
  }

  bool Equals(const WordContainer<N>& w) const {
    bool result = true;
    for (int i = 0; i < N; ++i) result &= (words_[i] == w.words_[i]);
    return result;
  }

  uint64_t Get(int start, int end) const {
    assert((0 <= start) && (start < (N * 64)));
    assert((start <= end) && (end < (N * 64)));
    assert((end - start) < 64);
    const int word0 = start / 64;
    const int word1 = end / 64;
    const int start_bit = start % 64;
    const int end_bit = end % 64;
    if (word0 == word1)
      return nldtool::GetWindowBits(words_[word0], start_bit,
                                    end_bit - start_bit + 1);
    const int len = 63 - start_bit + 1;
    return nldtool::GetWindowBits(words_[word0], start_bit, len) |
           (nldtool::GetWindowBits(words_[word1], 0, end_bit + 1) << len);
  }

  void Set(int start, int end, const uint64_t& cond) {
    assert((0 <= start) && (start < (N * 64)));
    assert((start <= end) && (end < (N * 64)));
    assert((end - start) < 64);
    const int word0 = start / 64;
    const int word1 = end / 64;
    const int start_bit = start % 64;
    const int end_bit = end % 64;
    if (word0 == word1)
      nldtool::SetWindowBits(words_[word0], start_bit, end_bit - start_bit + 1,
                             cond);
    else {
      const int len = 63 - start_bit + 1;
      nldtool::SetWindowBits(words_[word0], start_bit, len,
                             cond & nldtool::Mask(63 - start_bit + 1));
      nldtool::SetWindowBits(words_[word1], 0, end_bit + 1, cond >> len);
    }
  }
};

#endif  // WORD_CONTAINER_H_

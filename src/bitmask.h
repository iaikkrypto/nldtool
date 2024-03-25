#ifndef BITMASK_H_
#define BITMASK_H_

#include <cstdint>
#include <ostream>
#include <random>
#include <string>
#include <vector>

#include "bitpos.h"

/*!
 * \brief A mask for words, enabling the bitwise selection of parts of a word
 * or multiple words.
 */
class Bitmask {
  struct WordMask {
    int word;
    uint64_t mask;
    bool operator<(const WordMask& a) const {
      return (word < a.word) || (word == a.word && mask < a.mask);
    }
  };

 public:
  Bitmask();
  Bitmask(int word_size);
  Bitmask(const Bitmask& s);
  virtual ~Bitmask();

  Bitmask& operator=(const Bitmask& s);
  Bitmask& operator&=(const Bitmask& rhs);
  Bitmask& operator|=(const Bitmask& rhs);

  bool Empty() const;
  int GetNumBitsSet() const;
  int GetNumWordsSet() const;

  std::vector<WordMask>::iterator begin() { return masks_.begin(); }
  std::vector<WordMask>::iterator end() { return masks_.end(); }
  std::vector<WordMask>::iterator ClearBits(std::vector<WordMask>::iterator it,
                                            uint64_t mask);
  std::vector<WordMask>::iterator SetBits(std::vector<WordMask>::iterator it,
                                          uint64_t mask);

  void ClearBits(int word, uint64_t mask);
  void SetBits(int word, uint64_t mask);
  void ClearBit(int bit);
  void SetBit(int bit);
  void ClearBit(int word, int bit);
  void SetBit(int word, int bit);
  void ClearBit(const Bitpos& pos);
  void SetBit(const Bitpos& pos);
  void ClearWord(int word);
  void SetWord(int word);
  void ClearAll();
  void SetAll(int num_words);

  uint64_t GetWordMask(int word) const;
  bool GetBit(const Bitpos& pos) const;
  Bitpos GetRandomBitpos(std::mt19937& rng) const;
  Bitpos First() const;
  std::vector<Bitpos> GetBitposList() const;

  void WriteMask(std::ostream& fs) const;
  void PrintMask(const std::string& file_name = "") const;

 private:
  bool CheckMask() const;

  int word_size_;
  uint64_t word_mask_;
  std::vector<WordMask> masks_;
};

#endif  // BITMASK_H_

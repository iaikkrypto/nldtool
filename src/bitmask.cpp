#include "bitmask.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <ostream>

#include "utils.h"

Bitmask::Bitmask() : word_size_(0), word_mask_(0), masks_() {}

Bitmask::Bitmask(int word_size)
    : word_size_(word_size), word_mask_(nldtool::Mask(word_size_)), masks_() {}

Bitmask::~Bitmask() {}

Bitmask::Bitmask(const Bitmask& s)
    : word_size_(s.word_size_), word_mask_(s.word_mask_), masks_(s.masks_) {}

Bitmask& Bitmask::operator=(const Bitmask& s) {
  word_size_ = s.word_size_;
  word_mask_ = s.word_mask_;
  masks_ = s.masks_;
  return *this;
}

Bitmask& Bitmask::operator&=(const Bitmask& rhs) {
  assert(word_size_ == rhs.word_size_);
  assert(CheckMask());
  assert(rhs.CheckMask());
  int last = !masks_.empty() ? masks_.back().word : 0;
  int rhs_last = !rhs.masks_.empty() ? rhs.masks_.back().word : 0;
  int max = std::max(last, rhs_last);
  for (int i = 0; i <= max; i++) ClearBits(i, ~rhs.GetWordMask(i));
  return *this;
}

Bitmask& Bitmask::operator|=(const Bitmask& rhs) {
  assert(word_size_ == rhs.word_size_);
  assert(CheckMask());
  assert(rhs.CheckMask());
  for (int i = 0; i < rhs.masks_.size(); i++)
    SetBits(rhs.masks_[i].word, rhs.masks_[i].mask);
  return *this;
}

bool Bitmask::Empty() const { return masks_.empty(); }

int Bitmask::GetNumWordsSet() const {
  assert(CheckMask());
  return masks_.size();
}

int Bitmask::GetNumBitsSet() const {
  assert(CheckMask());
  int hw = 0;
  for (int i = 0; i < masks_.size(); i++)
    hw += nldtool::HammingWeight(masks_[i].mask);
  return hw;
}

bool Bitmask::CheckMask() const {
  for (int i = 0; i < masks_.size(); ++i)
    if (masks_[i].mask == 0 || masks_[i].word < 0) return false;
  return true;
}

std::vector<Bitmask::WordMask>::iterator Bitmask::ClearBits(
    std::vector<Bitmask::WordMask>::iterator it, uint64_t mask) {
  assert(masks_.begin() <= it && it < masks_.end());
  assert(CheckMask());
  mask &= word_mask_;
  it->mask &= ~mask;
  if (!it->mask) it = masks_.erase(it) - 1;
  return it;
}

std::vector<Bitmask::WordMask>::iterator Bitmask::SetBits(
    std::vector<Bitmask::WordMask>::iterator it, uint64_t mask) {
  assert(masks_.begin() <= it && it < masks_.end());
  assert(CheckMask());
  mask &= word_mask_;
  it->mask |= mask;
  return it;
}

void Bitmask::ClearBits(int word, uint64_t mask) {
  assert(word >= 0);
  assert(CheckMask());
  const WordMask wm = {word, 0};
  auto it = std::lower_bound(masks_.begin(), masks_.end(), wm);
  if (it != masks_.end() && it->word == word) ClearBits(it, mask);
}

void Bitmask::SetBits(int word, uint64_t mask) {
  assert(word >= 0);
  assert(CheckMask());
  mask &= word_mask_;
  if (!mask) return;
  const WordMask wm = {word, 0};
  auto it = std::lower_bound(masks_.begin(), masks_.end(), wm);
  if (it != masks_.end() && it->word == word)
    SetBits(it, mask);
  else
    masks_.insert(it, {word, mask});
}

void Bitmask::ClearBit(int bit) {
  ClearBit(bit / word_size_, bit % word_size_);
}

void Bitmask::SetBit(int bit) { SetBit(bit / word_size_, bit % word_size_); }

void Bitmask::ClearBit(int word, int bit) {
  assert(bit >= 0 && bit < word_size_);
  ClearBits(word, 1ull << bit);
}

void Bitmask::SetBit(int word, int bit) {
  assert(bit >= 0 && bit < word_size_);
  SetBits(word, 1ull << bit);
}

void Bitmask::ClearBit(const Bitpos& pos) {
  ClearBit(pos.GetWord(), pos.GetBit());
}

void Bitmask::SetBit(const Bitpos& pos) { SetBit(pos.GetWord(), pos.GetBit()); }

void Bitmask::ClearWord(int word) { ClearBits(word, word_mask_); }

void Bitmask::SetWord(int word) { SetBits(word, word_mask_); }

void Bitmask::SetAll(int num_words) {
  masks_.reserve(num_words);
  for (int i = 0; i < num_words; ++i) masks_.push_back({i, word_mask_});
}

void Bitmask::ClearAll() { masks_.clear(); }

uint64_t Bitmask::GetWordMask(int word) const {
  assert(word >= 0);
  assert(CheckMask());
  const WordMask wm = {word, 0};
  auto it = std::lower_bound(masks_.begin(), masks_.end(), wm);
  if (it != masks_.end() && it->word == word)
    return it->mask;
  else
    return 0;
}

bool Bitmask::GetBit(const Bitpos& pos) const {
  assert(pos.GetWord() >= 0);
  assert(pos.GetBit() >= 0 && pos.GetBit() < word_size_);
  return (GetWordMask(pos.GetWord()) >> pos.GetBit()) & 1;
}

Bitpos Bitmask::GetRandomBitpos(std::mt19937& rng) const {
  assert(CheckMask());
  assert(!Empty());
  std::uniform_int_distribution<int> dist(0, GetNumBitsSet() - 1);
  int index = dist(rng);
  int tmp = 0;
  for (int i = 0; i < masks_.size(); i++) {
    tmp += nldtool::HammingWeight(masks_[i].mask);
    if (index < tmp) {
      tmp -= nldtool::HammingWeight(masks_[i].mask);
      for (int j = 0; j < sizeof(uint64_t) * 8; ++j)
        if ((masks_[i].mask >> j & 1) && tmp++ == index)
          return Bitpos(masks_[i].word, j);
    }
  }
  assert(!"error");
  return Bitpos(-1, -1);
}

Bitpos Bitmask::First() const {
  assert(!Empty());
  assert(CheckMask());
  for (int j = 0; j < 64; ++j)
    if ((masks_[0].mask >> j) & 1) return Bitpos(masks_[0].word, j);
  assert(!"error");
  return Bitpos(-1, -1);
}

std::vector<Bitpos> Bitmask::GetBitposList() const {
  assert(CheckMask());
  std::vector<Bitpos> list;
  for (int i = 0; i < masks_.size(); i++)
    for (int j = 0; j < sizeof(uint64_t) * 8; ++j)
      if ((masks_[i].mask >> j) & 1) list.push_back(Bitpos(masks_[i].word, j));
  return list;
}

void Bitmask::WriteMask(std::ostream& fs) const {
  for (int i = 0; i < masks_.size(); i++) {
    fs << masks_[i].word << "\t";
    for (int j = word_size_ - 1; j >= 0; j--)
      fs << ((masks_[i].mask >> j) & 1ull);
    fs << std::endl;
  }
}

void Bitmask::PrintMask(const std::string& file_name) const {
  if (file_name.compare("") == 0) {
    WriteMask(std::cout);
  } else {
    std::fstream fs;
    fs.open(file_name.c_str(), std::ios::out);
    WriteMask(fs);
    fs.close();
  }
}

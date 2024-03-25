#include "bit_matrix.h"

#include <iostream>
#include <string>

#include "utils.h"

static const char LogTable256[256] = {
#define LT(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n
    -1,    0,     1,     1,     2,     2,     2,     2,     3,     3,     3,
    3,     3,     3,     3,     3,     LT(4), LT(5), LT(5), LT(6), LT(6), LT(6),
    LT(6), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7), LT(7)};

// --- construction ---

BitMatrix::BitMatrix(int rows, int cols)
    : rows_(rows),
      cols64_(cols / 64 + (cols % 64 > 0)),
      data_(rows_ * cols64_, 0),
      row_(rows_, {0, 0}),
      rowinv_(rows_),
      NOPIVOT(64 * cols64_) {
  for (int i = 0; i < rows_; ++i) {
    row_[i] = {i * cols64_, NOPIVOT};
  }
  Init();
}

BitMatrix::BitMatrix(const BitMatrix& cp)
    : rows_(cp.rows_),
      cols64_(cp.cols64_),
      data_(cp.data_),
      row_(cp.row_),
      rowinv_(cp.rowinv_),
      NOPIVOT(cp.NOPIVOT) {}

BitMatrix& BitMatrix::operator=(const BitMatrix& cp) {
  assert(rows_ == cp.rows_);
  assert(cols64_ == cp.cols64_);
  data_ = cp.data_;
  row_ = cp.row_;
  rowinv_ = cp.rowinv_;
  return *this;
}

// --- modification ---

void BitMatrix::SortByPivot(std::set<int>& changed, Permutation* perm,
                            bool /*backtrack*/) {
  if (changed.empty()) return;
  perm->start = *(changed.begin());
  perm->to.reserve(changed.size());
  perm->from.reserve(changed.size());
  for (int index : changed) perm->from.emplace_back(index, row_[index]);
  std::vector<Row> to_row;  // content of rows that are moved, sorted by pivot
                            // (=order of insertion)
  to_row.reserve(changed.size());
  for (int index : changed) to_row.emplace_back(row_[index]);
  std::sort(to_row.begin(), to_row.end(), Row());
  std::set<int>::const_iterator remove = changed.cbegin(),
                                removeend = changed.cend();
  std::vector<Row>::const_iterator insert = to_row.cbegin(),
                                   insertend = to_row.cend();

  int writeindex, readindex;
  for (writeindex = readindex = (*remove);
       writeindex != GetRows() && insert != insertend; ++writeindex) {
    if (writeindex == readindex && remove != removeend)
      writeindex = readindex = (*remove);
    while (remove != removeend && readindex == (*remove)) {
      ++remove;
      ++readindex;
    }
    if (readindex == GetRows() ||
        (*insert).pivot <=
            row_[readindex].pivot) {  // insert row from extracted list
      row_[writeindex] = (*insert);
      perm->to.emplace_back(writeindex, *insert);
      ++insert;
    } else {  // insert next row (that is not removed)
      row_[writeindex] = row_[readindex];
      ++readindex;
    }
    rowinv_[GetPhysRowIndex(writeindex)] = writeindex;
  }
  perm->end = writeindex;
  changed.clear();
}

void BitMatrix::ApplyPermutation(const Permutation* perm, bool inverse) {
  assert(perm);
  if (perm->from.empty()) return;
  assert(perm->from.size() == perm->to.size());
  std::vector<std::pair<int, Row>>::const_iterator remove, removeend, insert,
      insertend;
  int step, end;
  if (!inverse) {
    step = 1;
    remove = perm->from.cbegin();
    removeend = perm->from.cend();
    insert = perm->to.cbegin();
    insertend = perm->to.cend();
    end = perm->end;
  } else {
    step = -1;
    remove = perm->to.cend() - 1;
    removeend = perm->to.cbegin() - 1;
    insert = perm->from.cend() - 1;
    insertend = perm->from.cbegin() - 1;
    end = perm->start - 1;
  }

  for (int writeindex = (*remove).first, readindex = writeindex;
       writeindex != end && insert != insertend; writeindex += step) {
    if (writeindex == readindex && remove != removeend)
      writeindex = readindex = (*remove).first;
    while (remove != removeend && readindex == (*remove).first) {
      remove += step;
      readindex += step;
    }
    if (writeindex == (*insert).first) {  // insert row from to-list
      row_[writeindex] = (*insert).second;
      insert += step;
    } else {  // insert next row (that is not removed)
      row_[writeindex] = row_[readindex];
      readindex += step;
    }
    rowinv_[GetPhysRowIndex(writeindex)] = writeindex;
  }
}

// --- getters ---

int BitMatrix::GetLast(int row) const {
  for (int n = cols64_ - 1; n >= 0; --n) {
    const uint64_t word = GetWord64(row, n);
    if (word)
      for (int b = 63; b >= 0; --b)
        if ((word >> b) & 1) return b + n * 64;
  }
  return -1;
}

std::vector<int> BitMatrix::GetBitsSet(int row) const {
  std::vector<int> bits;
  bits.reserve(cols64_);
  for (int col = 0; col < cols64_ * 64; ++col)
    if (GetBit(row, col)) bits.push_back(col);
  return bits;
}

int BitMatrix::GetPivotRowCount() const {
  int ret = 0;
  for (int i = 0; i < rows_; ++i) {
    if (row_[i].pivot != NOPIVOT) ret++;
  }
  return ret;
}

void BitMatrix::Print() const {
  for (int i = 0; i < rows_; ++i) {
    if (row_[i].pivot == NOPIVOT) continue;
    std::cout << "(" << i << "," << row_[i].index / cols64_ << ","
              << row_[i].pivot << "): ";
    for (int j = 0; j < cols64_; ++j)
      for (int k = 0; k < 64; ++k)
        if ((data_[row_[i].index + j] >> k) & 1)
          std::cout << (j * 64 + k) << " ";
    std::cout << std::endl;
  }
}

void BitMatrix::PrintMatrix() const {
  for (int i = 0; i < rows_; ++i) {
    if (row_[i].pivot == NOPIVOT) continue;
    for (int j = 0; j < cols64_; ++j)
      for (int k = 0; k < 64; ++k) {
        if ((data_[row_[i].index + j] >> k) & 1)
          std::cout << "1";
        else
          std::cout << ".";
        if (k % 8 == 7) std::cout << " ";
      }
    std::cout << " (" << i << "," << row_[i].index / cols64_ << ","
              << row_[i].pivot << ")";
    std::cout << std::endl;
  }
}

// --- private helpers ---

int BitMatrix::ComputePivot(const uint64_t* row, int cols64) const {
  uint64_t lg = 0, t, tt;
  for (int n = 0; n < cols64; ++n) {
    const uint64_t word = row[n];
    if (word) {
      const uint64_t mask = nldtool::LeastSignificantSetBitmask(word);
      if ((tt = mask) & 0xFFFFull)
        lg = ((t = tt) & 0xFFull) ? LogTable256[t] : 8 + LogTable256[tt >> 8];
      else if ((tt = (mask >> 16)) & 0xFFFFull)
        lg = ((t = tt) & 0xFFull) ? 16 + LogTable256[t]
                                  : 24 + LogTable256[tt >> 8];
      else if ((tt = (mask >> 32)) & 0xFFFFull)
        lg = ((t = tt) & 0xFFull) ? 32 + LogTable256[t]
                                  : 40 + LogTable256[tt >> 8];
      else if ((tt = (mask >> 48)) & 0xFFFFull)
        lg = ((t = tt) & 0xFFull) ? 48 + LogTable256[t]
                                  : 56 + LogTable256[tt >> 8];
      else
        assert(!"log error");
      return lg + n * 64;
    }
  }
  return NOPIVOT;
}

#ifndef BIT_MATRIX_H_
#define BIT_MATRIX_H_

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <set>
#include <vector>

#include "change.h"
#include "row.h"

/*!
 * \brief The class that represents a matrix and provides functions for linear
 * calculations.
 */
class BitMatrix {
 public:
  // --- construction ---
  BitMatrix(int rows, int cols);
  BitMatrix(const BitMatrix& cp);
  BitMatrix& operator=(const BitMatrix& cp);

  // --- modification ---

  void Init() {
    for (int i = 0; i < rows_; ++i) {
      rowinv_[i] = i;
    }
  }

  bool SetBit(int row, int col) {
    assert(0 <= row && row < rows_);
    assert(0 <= col && col < cols64_ * 64);
    data_[row_[row].index + col / 64] |= 0x1ull << (col % 64);
    if (col < row_[row].pivot) {
      row_[row].pivot = col;
      return true;
    }
    return false;
  }

  bool XorBit(int row, int col) {
    assert(0 <= row && row < rows_);
    assert(0 <= col && col < cols64_ * 64);
    data_[row_[row].index + col / 64] ^= 0x1ull << (col % 64);
    if (col < row_[row].pivot) {
      row_[row].pivot = col;
      return true;
    }
    if (col == row_[row].pivot) {
      ComputePivot(row);
      return true;
    }
    return false;
  }

  bool XorRow(int row, const std::vector<int>& equation) {
    assert(std::is_sorted(equation.begin(), equation.end()));
    bool result = (row_[row].pivot >= equation[0]);
    uint64_t* rowdata = &data_[row_[row].index];
    for (int col : equation) rowdata[col / 64] ^= 0x1ull << (col % 64);
    if (result) {
      if (row_[row].pivot == equation[0])
        ComputePivot(row);
      else
        row_[row].pivot = equation[0];
    }
    return result;
  }

  bool XorRowtoRow(int a, int b) {
    assert(0 <= a && a < rows_);
    assert(0 <= b && b < rows_);
    for (int i = 0; i < cols64_; ++i)
      data_[row_[a].index + i] ^= data_[row_[b].index + i];
    if (row_[b].pivot < row_[a].pivot) {
      row_[a].pivot = row_[b].pivot;
      return true;
    }
    if (row_[a].pivot == row_[b].pivot) {
      ComputePivot(a);
      return true;
    }
    return false;
  }

  void SortByPivot(std::set<int>& changed, Permutation* perm,
                   bool backtrack = false);

  void ApplyPermutation(const Permutation* perm, bool inverse);

  // --- getters ---

  uint64_t GetWord64(int row, int col64) const {
    assert(0 <= row && row < rows_);
    assert(0 <= col64 && col64 < cols64_);
    return data_[row_[row].index + col64];
  }

  bool GetBit(int row, int col) const {
    assert(0 <= row && row < rows_);
    assert(0 <= col && col < cols64_ * 64);
    return (data_[row_[row].index + col / 64] >> (col % 64)) & 1;
  }

  int GetPivot(int row) const { return row_[row].pivot; }

  int GetRows() const { return rows_; }

  int GetColumns64() const { return cols64_; }

  int GetPhysRowIndex(int row) { return row_[row].index / cols64_; }

  int GetVirtRowIndex(int row) { return rowinv_[row]; }

  int GetPivotUpperBound(int col) const {
    const Row row = {0, col};
    const auto it = std::upper_bound(row_.begin(), row_.end(), row, Row());
    return std::distance(row_.begin(), it);
  }

  bool IsZeroRow(int row) const { return (row_[row].pivot == NOPIVOT); }

  int GetLast(int row) const;

  std::vector<int> GetBitsSet(int row) const;

  void Print() const;

  void PrintMatrix() const;

  int GetPivotRowCount() const;

 private:
  // --- private helpers ---

  void ComputePivot(int row) const {
    int& pivot = const_cast<int&>(row_[row].pivot);
    pivot = ComputePivot(&data_[row_[row].index], cols64_);
  }

  int ComputePivot(const uint64_t* row, int cols64) const;

  int rows_;
  int cols64_;
  std::vector<uint64_t> data_;
  std::vector<Row> row_;
  std::vector<int> rowinv_;

 public:
  const int NOPIVOT;
};

#endif  // BIT_MATRIX_H_

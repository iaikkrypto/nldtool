#ifndef LINEAR_MATRIX_H_
#define LINEAR_MATRIX_H_

#include <cstdint>
#include <set>
#include <vector>

#include "bit_matrix.h"
#include "change.h"
#include "tinyset.h"

/*!
 * \brief A class describing the matrix for the LinearStep and allowing
 * operation on the equation data.
 */
class LinearMatrix {
 public:
  // --- construction ---

  LinearMatrix(int rows, int cols);
  LinearMatrix(const LinearMatrix& cp);
  LinearMatrix& operator=(const LinearMatrix& cp);

  void Undo();

  void UndoAndRestore();

  void FixChanges();

  // --- modification ---

  void InitBit(int row, int col) {
    // does NOT guarantee reduced row echelon form,
    // use only for initializing matrix!
    // not logged as change
    matrix_.XorBit(row, col);
  }

  bool EpsilonGauss(bool backtrack = false);

  bool AddEquation(std::vector<int> equation, bool c, bool backtrack = false);

  // --- getters ---

  uint64_t GetWord64(int row, int col64) const {
    return matrix_.GetWord64(row, col64);
  }

  bool GetRhs(int row) const { return matrix_.GetBit(row, RHS_); }

  int GetPivot(int row) const { return matrix_.GetPivot(row); }

  int GetRows() const { return matrix_.GetRows(); }

  int GetColumns64() const {
    return matrix_.GetColumns64() - 1;  // to exclude rhs
  }

  int GetVirtRowIndex(int row) { return matrix_.GetVirtRowIndex(row); }

  int GetPhysRowIndex(int row) { return matrix_.GetPhysRowIndex(row); }

  TinySet<int>& GetRowsToExtract() { return rows_to_extract_; }

  bool IsZeroRow(int row) const { return matrix_.IsZeroRow(row); }

  bool IsContradiction(int row) const { return matrix_.GetPivot(row) == RHS_; }

  bool IsRowEchelon() const;

  bool IsReducedRowEchelon() const;

  int GetLast(int row) const { return matrix_.GetLast(row); }

  std::vector<int> GetBitsSet(int row) const { return matrix_.GetBitsSet(row); }

  int GetPivotRowCount() const { return matrix_.GetPivotRowCount(); };

  void Print() const { matrix_.Print(); }

  void PrintMatrix() const { matrix_.PrintMatrix(); }

 private:
  void Touch(int row, bool pivotchanged) {
    update_rows_.insert(row);
    if (pivotchanged) update_pivot_.insert(row);
  }

  // --- modification - log changes! ---
  friend class InitChange;
  void Init(bool log = true) {
    if (log) changes_.AddChange(new InitChange());
    int rows = matrix_.GetRows();
    rows_to_extract_.clear();
    rows_to_extract_.reserve(rows);
    for (int i = 0; i < rows; ++i) rows_to_extract_.push_back(i);
  }

  friend class XorBitChange;
  void XorBit(int row, int col, bool log = true) {
    if (log) changes_.AddChange(new XorBitChange(row, col));
    rows_to_extract_.insert(
        matrix_.GetPhysRowIndex(row));  // TODO move to touch!!!
    matrix_.XorBit(row, col);
  }

  friend class XorRowChange;
  void XorRow(int row, const std::vector<int>& equation, bool log = true) {
    if (log) changes_.AddChange(new XorRowChange(row, equation));
    rows_to_extract_.insert(matrix_.GetPhysRowIndex(row));
    matrix_.XorRow(row, equation);
  }

  friend class XorRowToRowChange;
  bool XorRowtoRow(int a, int b, bool log = true) {
    if (log) changes_.AddChange(new XorRowToRowChange(a, b));
    rows_to_extract_.insert(matrix_.GetPhysRowIndex(a));
    return matrix_.XorRowtoRow(a, b);
  }

  friend class SortChange;
  void SortByPivot(bool log = true) {
    Permutation perm;
    matrix_.SortByPivot(update_pivot_, &perm, log);
    if (log) changes_.AddChange(new SortChange(perm));
  }

  void ApplyPermutation(const Permutation* perm, bool inverse,
                        bool log = true) {
    matrix_.ApplyPermutation(perm, inverse);
    if (log) changes_.AddChange(new SortChange(*perm));
  }

  BitMatrix matrix_;
  int RHS_;
  // std::stack<ChangeList> changes_;
  ChangeList changes_;
  ChangeList stash_;
  TinySet<int> rows_to_extract_;
  std::set<int> update_rows_;
  std::set<int> update_pivot_;
};

#endif  // LINEAR_MATRIX_H_

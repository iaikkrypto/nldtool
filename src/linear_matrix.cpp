#include "linear_matrix.h"

#include <iostream>

//#define DEBUG_LINEARMATRIX

LinearMatrix::LinearMatrix(int rows, int cols)
    : matrix_(rows, cols + 64),
      RHS_(cols + 63),
      changes_(),
      stash_(),
      rows_to_extract_(),
      update_rows_(),
      update_pivot_() {
  Init(true);
}

LinearMatrix::LinearMatrix(const LinearMatrix& cp)
    : matrix_(cp.matrix_),
      RHS_(cp.RHS_),
      changes_(),
      stash_(),
      rows_to_extract_(cp.rows_to_extract_),
      update_rows_(cp.update_rows_),
      update_pivot_(cp.update_pivot_) {}

LinearMatrix& LinearMatrix::operator=(const LinearMatrix& cp) {
  matrix_ = cp.matrix_;
  RHS_ = cp.RHS_;
  changes_.Clear();
  stash_.Clear();
  rows_to_extract_ = cp.rows_to_extract_;
  update_rows_ = cp.update_rows_;
  update_pivot_ = cp.update_pivot_;
  return *this;
}

void LinearMatrix::Undo() {
  update_rows_.clear();
  update_pivot_.clear();
  changes_.Undo(this);
  stash_.Clear();
  stash_.Swap(changes_);
}

void LinearMatrix::UndoAndRestore() {
  update_rows_.clear();
  update_pivot_.clear();
  changes_.Undo(this);
  stash_.Apply(this);
  changes_.Clear();
  stash_.Clear();
}

void LinearMatrix::FixChanges() { changes_.Clear(); }

// --- modification ---

bool LinearMatrix::EpsilonGauss(bool backtrack) {
  while (!update_rows_.empty()) {
    const int row = *(update_rows_.begin());
    update_rows_.erase(update_rows_.begin());
    const int pivot = matrix_.GetPivot(row);
    if (IsContradiction(row)) return false;
    if (pivot == matrix_.NOPIVOT) continue;
    for (int i = 0; i < matrix_.GetRows(); i++) {
      if (i == row) continue;
      if (matrix_.GetBit(i, pivot)) {
        if (XorRowtoRow(i, row, backtrack)) {
          Touch(i, true);
        }
        if (IsContradiction(i)) return false;
      }
    }
  }
  SortByPivot(backtrack);
#ifdef DEBUG_LINEARMATRIX
  assert(IsReducedRowEchelon());
#endif
  return true;
}

bool LinearMatrix::AddEquation(std::vector<int> equation, bool c,
                               bool backtrack) {
  assert(std::is_sorted(equation.begin(), equation.end()));
#ifdef DEBUG_LINEARMATRIX
  assert(IsReducedRowEchelon());
#endif
  assert(!equation.empty());
  if (c) equation.push_back(RHS_);
  int row = matrix_.GetPivotUpperBound(equation[0]) - 1;
  bool is_pivot = (row >= 0 && GetPivot(row) == equation[0]);
  if (is_pivot) {
    // equation[0] is a pivot element
    XorRow(row, equation, backtrack);
    Touch(row, true);
    for (int i = 1; i < equation.size(); ++i) {
      int r = matrix_.GetPivotUpperBound(equation[i]) - 1;
      if (r >= 0 && row != r && GetPivot(r) == equation[i])
        XorRowtoRow(row, r, backtrack);
    }
  } else {
    // equation[0] is not a pivot element
    for (int r = 0; r <= row; ++r)
      if (matrix_.GetBit(r, equation[0])) XorRow(r, equation, backtrack);
    for (int i = 1; i < equation.size(); ++i) {
      int r = matrix_.GetPivotUpperBound(equation[i]) - 1;
      if (r >= 0 && GetPivot(r) == equation[i]) Touch(r, false);
    }
  }
  if (!update_rows_.empty()) return EpsilonGauss(backtrack);
#ifdef DEBUG_LINEARMATRIX
  assert(IsReducedRowEchelon());
#endif
  return true;
}

bool LinearMatrix::IsRowEchelon() const {
  int pivot = -1;
  for (int i = 0; i < matrix_.GetRows(); ++i)
    if (matrix_.GetPivot(i) >= pivot)
      pivot = matrix_.GetPivot(i);
    else {
      std::cout << "error: " << i << "<" << pivot << std::endl;
      return false;
    }
  return true;
}

bool LinearMatrix::IsReducedRowEchelon() const {
  if (!IsRowEchelon()) return false;
  for (int i = 0; i < matrix_.GetRows(); ++i) {
    int pivot = matrix_.GetPivot(i);
    if (pivot == matrix_.NOPIVOT) continue;
    for (int j = i - 1; j >= 0; --j)
      if (matrix_.GetBit(j, pivot)) {
        std::cout << "error: (" << j << "," << pivot << ")" << std::endl;
        return false;
      }
  }
  return true;
}

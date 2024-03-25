#include "change.h"

#include "linear_matrix.h"

// --- helper class : InitChange ---
void InitChange::Apply(LinearMatrix* matrix) const { matrix->Init(false); }

void InitChange::Undo(LinearMatrix* matrix) const { matrix->Init(false); }

// --- helper class : SortChange ---
void SortChange::Apply(LinearMatrix* matrix) const {
  matrix->ApplyPermutation(&perm, false, false);
}

void SortChange::Undo(LinearMatrix* matrix) const {
  matrix->ApplyPermutation(&perm, true, false);
}

// --- helper class : XorRowChange ---
void XorRowChange::Apply(LinearMatrix* matrix) const {
  matrix->XorRow(row, equation, false);
}

void XorRowChange::Undo(LinearMatrix* matrix) const {
  matrix->XorRow(row, equation, false);
}

// --- helper class : XorRowChange ---
void XorRowToRowChange::Apply(LinearMatrix* matrix) const {
  matrix->XorRowtoRow(rowa, rowb, false);
}

void XorRowToRowChange::Undo(LinearMatrix* matrix) const {
  matrix->XorRowtoRow(rowa, rowb, false);
}

// --- helper class : XorBitChange ---
void XorBitChange::Apply(LinearMatrix* matrix) const {
  matrix->XorBit(row, col, false);
}

void XorBitChange::Undo(LinearMatrix* matrix) const {
  matrix->XorBit(row, col, false);
}

// --- helper class : ChangeList ---
void ChangeList::AddChange(Change* change) {
  list.push_back(std::unique_ptr<Change>(change));
}

void ChangeList::Apply(LinearMatrix* matrix) {
  for (const std::unique_ptr<Change>& change : list) change->Apply(matrix);
}

void ChangeList::Undo(LinearMatrix* matrix) {
  for (auto it = list.rbegin(); it != list.rend(); ++it) (*it)->Undo(matrix);
}

void ChangeList::Clear() { list.clear(); }

void ChangeList::Swap(ChangeList& cp) { list.swap(cp.list); }

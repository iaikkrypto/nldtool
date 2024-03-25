#ifndef CHANGE_H
#define CHANGE_H

#include <memory>
#include <vector>

#include "row.h"

class LinearMatrix;

/*!
 * \brief A helper class describing a permutation in the matrix.
 */
struct Permutation {
  int start;
  int end;
  // list of moved Rows, sorted
  std::vector<std::pair<int, Row>> from;
  // list of target indices and corresponding rows to be inserted, sorted
  std::vector<std::pair<int, Row>> to;
};

/*!
 * \brief Base helper class describing a change in the matrix.
 */
class Change {
 public:
  virtual ~Change() {}
  virtual void Apply(LinearMatrix* matrix) const {}
  virtual void Undo(LinearMatrix* matrix) const {}
};

/*!
 * \brief A class describing the change of the initialization.
 */
class InitChange : public Change {
 public:
  InitChange() {}
  virtual void Apply(LinearMatrix* matrix) const;
  virtual void Undo(LinearMatrix* matrix) const;
};

/*!
 * \brief A class describing the change of a call to a sorting algorithm.
 */
class SortChange : public Change {
 public:
  SortChange(const Permutation& perm) : perm(perm) {}
  virtual void Apply(LinearMatrix* matrix) const;
  virtual void Undo(LinearMatrix* matrix) const;
  Permutation perm;
};

/*!
 * \brief Helper class for a xor row change.
 */
class XorRowChange : public Change {
 public:
  XorRowChange(int row, const std::vector<int>& equation)
      : row(row), equation(equation) {}
  virtual void Apply(LinearMatrix* matrix) const;
  virtual void Undo(LinearMatrix* matrix) const;
  int row;
  std::vector<int> equation;
};

/*!
 * \brief Helper class for a xor row to row change.
 */
class XorRowToRowChange : public Change {
 public:
  XorRowToRowChange(int rowa, int rowb) : rowa(rowa), rowb(rowb) {}
  virtual void Apply(LinearMatrix* matrix) const;
  virtual void Undo(LinearMatrix* matrix) const;
  int rowa, rowb;
};

/*!
 * \brief Helper class for a xor bit change.
 */
class XorBitChange : public Change {
 public:
  XorBitChange(int row, int col) : row(row), col(col) {}
  virtual void Apply(LinearMatrix* matrix) const;
  virtual void Undo(LinearMatrix* matrix) const;
  int row, col;
};

/*!
 * \brief A collection of changes, with the option to apply or undo them all.
 */
class ChangeList {
 public:
  void AddChange(Change* change);
  void Apply(LinearMatrix* matrix);
  void Undo(LinearMatrix* matrix);
  void Clear();
  void Swap(ChangeList& cp);

 private:
  std::vector<std::unique_ptr<Change>> list;
};

#endif  // CHANGE_H

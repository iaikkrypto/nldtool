#include "linear_conditions.h"

#include <cassert>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <set>
#include <vector>

#include "bitpos.h"
#include "characteristic.h"
#include "step.h"

//#define DEBUG_TRANSFER

LinearConditions::LinearConditions(int rows, int cols)
    : initial_f_count(rows * VARSPERBIT),
      linear_matrix_(new LinearMatrix(rows * VARSPERBIT, cols * VARSPERBIT)) {
  equation_to_bitcondition_[0] = BitCondition("?");
  equation_to_bitcondition_[1] = BitCondition("#");
  equation_to_bitcondition_[2] = BitCondition("5");
  equation_to_bitcondition_[3] = BitCondition("A");
  equation_to_bitcondition_[4] = BitCondition("3");
  equation_to_bitcondition_[5] = BitCondition("C");
  equation_to_bitcondition_[6] = BitCondition("-");
  equation_to_bitcondition_[7] = BitCondition("x");
  // only if nonlinear conditions are used
  equation_to_bitcondition_[8] = BitCondition("7");
  equation_to_bitcondition_[9] = BitCondition("?");
  equation_to_bitcondition_[10] = BitCondition("D");
  equation_to_bitcondition_[11] = BitCondition("?");
  equation_to_bitcondition_[12] = BitCondition("B");
  equation_to_bitcondition_[13] = BitCondition("?");
  equation_to_bitcondition_[14] = BitCondition("?");
  equation_to_bitcondition_[15] = BitCondition("E");
}

BitCondition LinearConditions::GetBitConditionFromRow(int row) const {
  const int pivot = linear_matrix_->GetPivot(row);
  const int col64 = pivot / 64;
  const int index = (pivot % 64) / VARSPERBIT;
  const uint64_t word = linear_matrix_->GetWord64(row, col64);
  if (word & ~(nldtool::Mask(VARSPERBIT) << (index * VARSPERBIT)))
    return BitCondition("?");
  for (int i = col64 + 1; i < linear_matrix_->GetColumns64(); ++i)
    if (linear_matrix_->GetWord64(row, i)) return BitCondition("?");
  const int cond = (word >> (index * VARSPERBIT)) & nldtool::Mask(VARSPERBIT);
  bool rhs = linear_matrix_->GetRhs(row);
  return equation_to_bitcondition_[(cond << 1) | rhs];
}

BitCondition LinearConditions::GetBitConditionFromRows(int row_a,
                                                       int row_b) const {
  if (row_a < 0 || row_a >= linear_matrix_->GetRows() || row_b < 0 ||
      row_b >= linear_matrix_->GetRows())
    return BitCondition("?");
  const int pivot_a = linear_matrix_->GetPivot(row_a);
  const int pivot_b = linear_matrix_->GetPivot(row_b);
  if (pivot_a / VARSPERBIT != pivot_b / VARSPERBIT) return BitCondition("?");
  const int col64 = pivot_a / 64;
  const int index = (pivot_a % 64) / VARSPERBIT;
  const uint64_t word = linear_matrix_->GetWord64(row_a, col64) ^
                        linear_matrix_->GetWord64(row_b, col64);
  if (word & ~(nldtool::Mask(VARSPERBIT) << (index * VARSPERBIT)))
    return BitCondition("?");
  for (int i = col64 + 1; i < linear_matrix_->GetColumns64(); ++i)
    if (linear_matrix_->GetWord64(row_a, i) ^
        linear_matrix_->GetWord64(row_b, i))
      return BitCondition("?");
  const int cond = (word >> (index * VARSPERBIT)) & nldtool::Mask(VARSPERBIT);
  bool rhs = linear_matrix_->GetRhs(row_a) ^ linear_matrix_->GetRhs(row_b);
  return equation_to_bitcondition_[(cond << 1) | rhs];
}

bool LinearConditions::PropagateConditions(Characteristic& characteristic,
                                           std::vector<ConditionWordPtr> params,
                                           int word_size) {
  BitCondition bc;
  //    int row = -1, row2 = -1;
  //    while((row = linear_matrix_->GetNextTouchedRow()) != -1) {
  int row2 = -1;

  for (int datarow : linear_matrix_->GetRowsToExtract()) {
    int row = linear_matrix_->GetVirtRowIndex(datarow);
    if (linear_matrix_->IsContradiction(row)) return false;
    if (linear_matrix_->IsZeroRow(row)) continue;
    if (linear_matrix_->IsContradiction(row)) return false;
    const int pivot = linear_matrix_->GetPivot(row);
    const int bit = pivot / VARSPERBIT;
    ConditionProxyPtr cp =
        params[params.size() - 1 - bit / word_size]->GetConditionProxy(
            bit % word_size);
    bc = GetBitConditionFromRow(row);
    bc.Filter(BitCondition(cp->GetCondition(characteristic)));
    // if (bc.GetValue() !=
    // BitCondition(cp->GetCondition(characteristic)).GetValue()) std::cout << "
    // " << row;
    if (!cp->SetCondition(characteristic, bc)) return false;
    if (row == row2 && (pivot % VARSPERBIT)) continue;
    row2 = (pivot % VARSPERBIT == 0) ? row + 1 : row - 1;
    bc = GetBitConditionFromRows(row, row2);
    bc.Filter(BitCondition(cp->GetCondition(characteristic)));
    // if (bc.GetValue() !=
    // BitCondition(cp->GetCondition(characteristic)).GetValue()) std::cout << "
    // " << row << "+" << row2;
    if (!cp->SetCondition(characteristic, bc)) return false;
  }
  linear_matrix_->GetRowsToExtract().clear();
  return true;
}

bool LinearConditions::TransferLinearConditions(
    LinearConditions& other_linear_conditions, Overlap overlap, int word_size) {
  for (int row = 0; row < linear_matrix_->GetRows(); ++row) {
    if (linear_matrix_->IsZeroRow(row)) continue;
    const int pivot = linear_matrix_->GetPivot(row);
    const int last = linear_matrix_->GetLast(row);
    const int start = overlap.start * word_size * VARSPERBIT;
    const int end = (overlap.start + overlap.size) * word_size * VARSPERBIT - 1;
    if (pivot < start || last > end) continue;
    std::vector<int> bits = linear_matrix_->GetBitsSet(row);
    bool c = linear_matrix_->GetRhs(row);
#ifdef DEBUG_TRANSFER
    std::cout << row << ": row(" << pivot << "," << last << ") overlap("
              << start << "," << end << ")" << std::endl;
    std::cout << "this: " << bits.size() << " bits set: ";
    for (int& x : bits) std::cout << x << ", ";
    std::cout << c << std::endl;
    std::cout << "this matrix (" << (&linear_matrix_) << "):" << std::endl;
    linear_matrix_->Print();
    std::cout << endl;
#endif
    for (int& x : bits)
      x = x - start + overlap.other_start * word_size * VARSPERBIT;
#ifdef DEBUG_TRANSFER
    std::cout << "other: " << bits.size() << " bits set: ";
    for (int& x : bits) std::cout << x << ", ";
    std::cout << c << std::endl;
    std::cout << "other matrix (" << (&(other_linear_conditions.GetMatrix()))
              << "):" << std::endl;
    other_linear_conditions.GetMatrix().Print();
    std::cout << endl;
#endif
    bool result = other_linear_conditions.GetMatrix().AddEquation(bits, c);
#ifdef DEBUG_TRANSFER
    std::cout << "other matrix (" << (&(other_linear_conditions.GetMatrix()))
              << "):" << std::endl;
    other_linear_conditions.GetMatrix().Print();
    if (result)
      std::cout << "transfer ok!" << std::endl;
    else
      std::cout << "transfer failed!" << std::endl;
    std::cout << endl;
#endif
    if (!result) return false;
  }
  return true;
}

bool LinearConditions::AddLinearConditions(BitCondition& bit_condition, int pos,
                                           bool backtrack) {
  int bit = VARSPERBIT * pos;
  switch (bit_condition.GetChar()) {
    case '0':
      return linear_matrix_->AddEquation({bit}, 0, backtrack) &&
             linear_matrix_->AddEquation({bit + 1}, 0, backtrack);
    case '1':
      return linear_matrix_->AddEquation({bit}, 1, backtrack) &&
             linear_matrix_->AddEquation({bit + 1}, 1, backtrack);
    case 'n':
      return linear_matrix_->AddEquation({bit}, 0, backtrack) &&
             linear_matrix_->AddEquation({bit + 1}, 1, backtrack);
    case 'u':
      return linear_matrix_->AddEquation({bit}, 1, backtrack) &&
             linear_matrix_->AddEquation({bit + 1}, 0, backtrack);
    case '-':
      return linear_matrix_->AddEquation({bit, bit + 1}, 0, backtrack);
    case 'x':
      return linear_matrix_->AddEquation({bit, bit + 1}, 1, backtrack);
    case '3':
      return linear_matrix_->AddEquation({bit + 1}, 0, backtrack);
    case '5':
      return linear_matrix_->AddEquation({bit}, 0, backtrack);
    case 'A':
      return linear_matrix_->AddEquation({bit}, 1, backtrack);
    case 'C':
      return linear_matrix_->AddEquation({bit + 1}, 1, backtrack);
  }
  return true;
}

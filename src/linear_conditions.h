#ifndef LINEAR_CONDITIONS_H_
#define LINEAR_CONDITIONS_H_

#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>

#include "condition.h"
#include "linear_matrix.h"
#include "step.h"

class Characteristic;
class ConditionWord;
typedef std::shared_ptr<ConditionWord> ConditionWordPtr;

/*!
 * \brief A class dealing with linear \see BitConditions and managing their
 * representation in a \see LinearMatrix.
 */
class LinearConditions {
 public:
  static const int VARSPERBIT = 2;
  int initial_f_count;

  LinearConditions(int rows, int cols);

  LinearConditions(const LinearConditions& cp)
      : initial_f_count(cp.initial_f_count),
        linear_matrix_(new LinearMatrix(*cp.linear_matrix_)) {
    memcpy(equation_to_bitcondition_, cp.equation_to_bitcondition_,
           16 * sizeof(BitCondition));
  }

  LinearConditions& operator=(const LinearConditions& cp) {
    initial_f_count = cp.initial_f_count;
    if (this != &cp &&
        linear_matrix_ == cp.linear_matrix_)  // both point to the same matrix,
                                              // but should have separate copies
      linear_matrix_ =
          std::shared_ptr<LinearMatrix>(new LinearMatrix(*cp.linear_matrix_));
    else
      linear_matrix_->operator=(*cp.linear_matrix_);
    return *this;
  }

  void ShallowCopy(const LinearConditions& cp) {
    linear_matrix_ = cp.linear_matrix_;
    linear_matrix_->FixChanges();
  }

  void Undo() { linear_matrix_->Undo(); }

  void UndoAndRestore() { linear_matrix_->UndoAndRestore(); }

  template <class F>
  void InitLinearFunction(int word_size) {
    const int offset = F::kNumOutputs * word_size * VARSPERBIT;
    // set matrix to [f(E)^t E]
    for (int i = 0; i < F::kNumOutputs; ++i)
      for (int j = 0; j < word_size; ++j) {
        const int pos = (i * word_size + j) * VARSPERBIT;
        linear_matrix_->InitBit(pos, pos);
        linear_matrix_->InitBit(pos + 1, pos + 1);
      }
    for (int i = 0; i < F::kNumInputs; ++i)
      for (int j = 0; j < word_size; ++j) {
        const int inpos =
            ((F::kNumInputs - 1 - i) * word_size + j) * VARSPERBIT;
        uint64_t args[F::kNumInputs + F::kNumOutputs] = {0};
        args[i] = 1ull << j;
        F::f(word_size, args, args + F::kNumInputs);
        for (int k = 0; k < F::kNumOutputs; ++k)
          for (int l = 0; l < word_size; ++l)
            if ((args[F::kNumInputs + k] >> l) & 1) {
              const int outpos =
                  ((F::kNumOutputs - 1 - k) * word_size + l) * VARSPERBIT;
              linear_matrix_->InitBit(outpos, offset + inpos);
              linear_matrix_->InitBit(outpos + 1, offset + inpos + 1);
            }
      }
  }

  BitCondition GetBitConditionFromRow(int row) const;
  BitCondition GetBitConditionFromRows(int row_a, int row_b) const;
  bool PropagateConditions(Characteristic& characteristic,
                           std::vector<ConditionWordPtr> params, int word_size);
  bool TransferLinearConditions(LinearConditions& other_linear_conditions,
                                Overlap overlap, int word_size);
  bool AddLinearConditions(BitCondition& bit_condition, int pos,
                           bool backtrack = false);

  void PrintMatrix() const { linear_matrix_->PrintMatrix(); }

  void Print() const { linear_matrix_->Print(); }

  LinearMatrix& GetMatrix() { return *linear_matrix_; }

  const LinearMatrix& GetMatrix() const { return *linear_matrix_; }

 private:
  std::shared_ptr<LinearMatrix> linear_matrix_;
  BitCondition equation_to_bitcondition_[16];
};

#endif  // LINEAR_CONDITIONS_H_

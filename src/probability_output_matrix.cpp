#include "probability_output_matrix.h"

#include <cassert>
#include <cmath>

ProbabilityOutputMatrix::ProbabilityOutputMatrix() : matrix_(), size_(0) {
  assert(size_ == 0);
}

ProbabilityOutputMatrix::ProbabilityOutputMatrix(int size)
    : matrix_(size * size, 0), size_(size) {}

ProbabilityOutputMatrix::~ProbabilityOutputMatrix() {}

ProbabilityOutputMatrix::ProbabilityOutputMatrix(
    const ProbabilityOutputMatrix& a)
    : matrix_(a.matrix_), size_(a.size_) {}

ProbabilityOutputMatrix& ProbabilityOutputMatrix::operator=(
    const ProbabilityOutputMatrix& a) {
  if (this == &a) return *this;
  matrix_ = a.matrix_;
  size_ = a.size_;
  return *this;
}

const ProbabilityOutputMatrix ProbabilityOutputMatrix::operator+(
    const ProbabilityOutputMatrix& other) const {
  assert(size_ == other.size_);
  ProbabilityOutputMatrix result = *this;
  for (int i = 0; i < matrix_.size(); ++i)
    result.matrix_[i] = matrix_[i] + other.matrix_[i];
  return result;
}

const ProbabilityOutputMatrix ProbabilityOutputMatrix::operator*(
    const ProbabilityOutputMatrix& other) const {
  assert(size_ == other.size_);
  ProbabilityOutputMatrix result(other.GetColums());
  for (int row = 0; row < size_ * size_; row += size_)
    for (int column = 0; column < size_; ++column)
      for (int j = 0, jsize = 0; j < size_; ++j, jsize += size_)
        result.matrix_[row + column] +=
            matrix_[row + j] * other.matrix_[jsize + column];
  return result;
}

void ProbabilityOutputMatrix::Set(int rows, int columns, float value) {
  assert(rows < size_ && columns < size_);
  matrix_[size_ * rows + columns] = value;
}

void ProbabilityOutputMatrix::Increment(int rows, int columns) {
  assert(rows < size_ && columns < size_);
  matrix_[size_ * rows + columns] += 1;
}

void ProbabilityOutputMatrix::Divide(float val) {
  if (val == 0)
    for (int i = 0; i < matrix_.size(); ++i) matrix_[i] = 0;
  else
    for (int i = 0; i < matrix_.size(); ++i) matrix_[i] /= val;
}

float ProbabilityOutputMatrix::Get(int rows, int columns) const {
  assert(rows < size_ && columns < size_);
  return matrix_[size_ * rows + columns];
}

int ProbabilityOutputMatrix::GetRows() const { return size_; }
int ProbabilityOutputMatrix::GetColums() const { return size_; }

std::ostream& operator<<(std::ostream& os, const ProbabilityOutputMatrix& t) {
  for (int i = 0; i < t.size_; ++i) {
    for (int j = 0; j < t.size_; ++j) os << t.matrix_[t.size_ * i + j];
    os << std::endl;
  }
  return os;
}

void ProbabilityOutputMatrix::ResortOutput(uint8_t perm[], int len) {}

const char* ProbabilityOutputMatrix::GetBytePtr() const {
  return reinterpret_cast<const char*>(matrix_.data());
}

int ProbabilityOutputMatrix::GetByteSize() const {
  return matrix_.size() * sizeof(float);
}

void ProbabilityOutputMatrix::SetFromBytePtr(const char* data, int size) {
  assert(size % sizeof(float) == 0);
  int count = size / sizeof(float);
  float* ptr = reinterpret_cast<float*>(const_cast<char*>(data));
  matrix_.assign(ptr, ptr + count);
  size_ = int(sqrt(count));
}

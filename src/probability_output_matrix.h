#ifndef PROBABILITY_OUTPUT_MATRIX_H_
#define PROBABILITY_OUTPUT_MATRIX_H_

#include <cstdint>
#include <iostream>
#include <vector>

/*!
 * \brief Data structure for the output of the \see ProbabilityMatrix action
 */
class ProbabilityOutputMatrix {
 public:
  std::vector<float> matrix_;
  int size_;

  ProbabilityOutputMatrix();
  ProbabilityOutputMatrix(int size);
  ~ProbabilityOutputMatrix();
  ProbabilityOutputMatrix(const ProbabilityOutputMatrix& a);
  ProbabilityOutputMatrix& operator=(const ProbabilityOutputMatrix& a);
  const ProbabilityOutputMatrix operator+(
      const ProbabilityOutputMatrix& other) const;
  const ProbabilityOutputMatrix operator*(
      const ProbabilityOutputMatrix& other) const;
  void Set(int rows, int columns, float value);
  void Increment(int rows, int columns);
  void Divide(float val);
  float Get(int rows, int columns) const;
  int GetRows() const;
  int GetColums() const;
  friend std::ostream& operator<<(std::ostream& os,
                                  const ProbabilityOutputMatrix& t);
  void ResortOutput(uint8_t perm[], int len);
  const char* GetBytePtr() const;
  int GetByteSize() const;
  void SetFromBytePtr(const char* data, int size);
};

#endif  // PROBABILITY_OUTPUT_MATRIX_H_

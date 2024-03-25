#ifndef ROW_H_
#define ROW_H_

#include <ostream>

/*!
 * \brief  Struct used to compare rows of a matrix
 */
struct Row {
  int index;
  int pivot;
  bool operator()(const Row& a, const Row& b) {
    // same as a < b, for usage with std::sort, upper_bound etc
    return a.pivot < b.pivot;
  }

  bool operator<(const Row& other) { return pivot < other.pivot; }

  friend std::ostream& operator<<(std::ostream& stream, const Row& row) {
    stream << "(" << row.index << "," << row.pivot << ")";
    return stream;
  }
};

#endif  // ROW_H_

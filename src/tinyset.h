#ifndef TINYSET_H_
#define TINYSET_H_

#include <algorithm>
#include <vector>

/*!
 * \brief A custom set implementation based on a vector, fast for small sets.
 */
template <typename T>
class TinySet {
 public:
  typename std::vector<T>::iterator begin() { return setdata.begin(); }
  typename std::vector<T>::const_iterator begin() const {
    return setdata.begin();
  }
  typename std::vector<T>::iterator end() { return setdata.end(); }
  typename std::vector<T>::const_iterator end() const { return setdata.end(); }

  void insert(const T& val) {
    if (find(val) == end()) setdata.push_back(val);
  }
  typename std::vector<T>::const_iterator find(const T& val) const {
    return std::find(begin(), end(), val);
  }
  void reserve(int size) { setdata.reserve(size); }
  void push_back(const T& val) { setdata.push_back(val); }
  void clear() { setdata.clear(); }

 private:
  std::vector<T> setdata;
};

#endif  // TINYSET_H_

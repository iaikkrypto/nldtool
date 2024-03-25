#ifndef INDEX_H_
#define INDEX_H_

/*!
 * \brief Helper class for indexing pairs for the calculations.
 */
template <int N>
class Index {
  int val_[N];
  int size_[N];

 public:
  Index() : val_(), size_() {}

  int GetSize(int i) { return size_[i]; }

  void SetSize(int i, int s) { size_[i] = s; }

  void AddToSize(int i, int s) { size_[i] += s; }

  int& operator[](int i) { return val_[i]; }

  int operator[](int i) const { return val_[i]; }

  bool Increment() {
    ++val_[0];
    for (int j = 0; j < N; ++j) {
      if (val_[j] == size_[j]) {
        if (j == N - 1) return false;
        val_[j] = 0;
        ++val_[j + 1];
      }
    }
    return true;
  }
};

#endif  // INDEX_H_

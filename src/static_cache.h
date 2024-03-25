#ifndef STATIC_CACHE_H_
#define STATIC_CACHE_H_

#ifndef MAX_STATIC_CACHE_SIZE
#define MAX_STATIC_CACHE_SIZE 16
#endif

#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>

#include "cache_base.h"

/*!
 * \brief Small cache for functions with small number of in- and outputs,
 * results are calculated once in the beginning.
 *
 * The maximum size of the static cache is controlled with the define
 * MAX_STATIC_CACHE_SIZE.
 */
template <class Key, class Data>
class StaticCache : public CacheBase<Key, Data> {
 public:
  StaticCache() { is_initialized_ = false; }

  virtual ~StaticCache() {}

  virtual void Clear() { data_.clear(); }

  virtual int64_t GetSize() { return data_.size(); }

  virtual void SetSize(int64_t size) { data_.resize(size); }

  virtual bool IsInitialized() { return is_initialized_; }

  virtual void SetInitialized() { is_initialized_ = true; }

  virtual void Insert(const Key key, const Data value) { data_[key] = value; }

  virtual typename CacheBase<Key, Data>::Match Find(const Key key) /*const*/ {
    typename CacheBase<Key, Data>::Match match;
    match.matched = true;
    match.first = key;
    match.second = data_[key];
    return match;
  }

  virtual bool LoadDump() {
    if (this->filepath == nullptr) return false;
    FILE* file = fopen(this->filepath, "r");
    if (file == nullptr) {
      // std::cout << "cache dump file "<< this->filepath << " not found" <<
      // std::endl;
      return false;
    }
    Data data;
    int vsiz = data.GetByteSize();
    char* vbuf = new char[vsiz];
    data_.clear();
    while (fread(vbuf, vsiz, 1, file) == 1) {
      data.SetFromBytePtr(vbuf, vsiz);
      data_.push_back(data);
    }
    std::cout << "reading cache dump file " << this->filepath
              << " was successful, new cache size: " << data_.size()
              << std::endl;
    delete[] vbuf;
    fclose(file);
    if (data_.size()) is_initialized_ = true;
    return true;
  }

  virtual bool SaveDump() {
    if (this->filepath == nullptr) return false;
    if (data_.size() <= 0) return true;
    FILE* file = fopen(this->filepath, "w");
    if (file == nullptr) return false;
    int vsiz = data_[0].GetByteSize();
    for (int i = 0; i < data_.size(); ++i) {
      const char* vbuf = data_[i].GetBytePtr();
      int res = fwrite(vbuf, vsiz, 1, file);
      if (res != 1) {
        fclose(file);
        std::cerr << "writing cache dump file " << this->filepath << " failed"
                  << std::endl;
        return false;
      }
    }
    std::cout << "writing cache dump file " << this->filepath
              << " was successful" << std::endl;
    fclose(file);
    return true;
  }

 private:
  typedef std::vector<Data> CacheMap;
  CacheMap data_;
  bool is_initialized_;
};

#endif  // STATIC_CACHE_H_

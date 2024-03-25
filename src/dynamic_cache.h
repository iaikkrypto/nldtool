#ifndef DYNAMIC_CACHE_H_
#define DYNAMIC_CACHE_H_

#ifndef MAX_DYNAMIC_CACHE_SIZE
#define MAX_DYNAMIC_CACHE_SIZE 22
#endif

#include <cassert>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <list>
#include <unordered_map>

#include "cache_base.h"

/*!
 * \brief The dynamic version of the cache, using a std::unordered_map.
 *
 * In this dynamic cache, the results are calculated when they are needed, and
 * stored in the cache for later access. The std::unordered_map's implementation
 * is a great performance gain for our case when compared to a std::map. The
 * maximum capacity of the dynamic cache is controlled via the define
 * MAX_DYNAMIC_CACHE_SIZE.
 */
template <class Key, class Data>
class DynamicCache : public CacheBase<Key, Data> {
 public:
  DynamicCache() { capacity_ = 1ull << MAX_DYNAMIC_CACHE_SIZE; }

  virtual ~DynamicCache() {}

  virtual void Insert(const Key key, const Data value) {
    if (data_.size() == capacity_) Evict();
    ListIt it = lru_data_.insert(lru_data_.begin(), key);
    std::pair<CacheMapIt, bool> pp =
        data_.insert(typename CacheMap::value_type(key, make_pair(value, it)));
    if (pp.second == false) lru_data_.pop_front();
  }

  virtual typename CacheBase<Key, Data>::Match Find(const Key key) /*const*/ {
    typename CacheBase<Key, Data>::Match match;
    match.matched = false;
    match.first = key;
    CacheMapIt iter = data_.find(key);
    if (iter != data_.end()) {
      match.matched = true;
      match.first = iter->first;
      match.second = iter->second.first;
      lru_data_.splice(lru_data_.begin(), lru_data_, iter->second.second);
      iter->second.second = lru_data_.begin();
    }
    return match;
  }

  virtual void Clear() { data_.clear(); }

  virtual int64_t GetSize() { return data_.size(); }

  virtual void SetSize(int64_t size) {}

  virtual bool IsInitialized() { return true; }

  virtual void SetInitialized() {}

  virtual bool LoadDump() {
    if (this->filepath == nullptr) return false;
    FILE* file = fopen(this->filepath, "r");
    if (file == nullptr) {
      // std::cout << "cache dump file " << this->filepath << " not found" <<
      // std::endl;
      return false;
    }
    Key key;
    Data data;
    int ksiz = key.GetByteSize();
    int vsiz = data.GetByteSize();
    char* kbuf = new char[ksiz];
    char* vbuf = new char[vsiz];
    data_.clear();
    while (fread(kbuf, ksiz, 1, file) == 1 && fread(vbuf, vsiz, 1, file) == 1) {
      key.SetFromBytePtr(kbuf, ksiz);
      data.SetFromBytePtr(vbuf, vsiz);
      Insert(key, data);
    }
    std::cout << "reading cache dump file " << this->filepath
              << " was successful, new cache size: " << data_.size()
              << std::endl;
    delete[] kbuf;
    delete[] vbuf;
    fclose(file);
    return true;
  }

  virtual bool SaveDump() {
    if (this->filepath == nullptr) return false;
    if (data_.size() <= 0) return true;
    FILE* file = fopen(this->filepath, "w");
    if (file == nullptr) return false;
    CacheMapIt it = data_.begin();
    int ksiz = it->first.GetByteSize();
    int vsiz = it->second.first.GetByteSize();
    for (; it != data_.end(); ++it) {
      const char* kbuf = it->first.GetBytePtr();
      const char* vbuf = it->second.first.GetBytePtr();
      if (fwrite(kbuf, ksiz, 1, file) != 1 ||
          fwrite(vbuf, vsiz, 1, file) != 1) {
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
  void Evict() {
    assert(!lru_data_.empty());
    CacheMapIt it = data_.find(lru_data_.back());
    assert(it != data_.end());
    data_.erase(it);
    lru_data_.pop_back();
  }

  typedef typename std::list<Key>::iterator ListIt;

  typedef std::unordered_map<Key, std::pair<Data, ListIt>, Key, Key> CacheMap;
  typedef typename std::unordered_map<Key, std::pair<Data, ListIt>, Key,
                                      Key>::iterator CacheMapIt;

  CacheMap data_;
  uint64_t capacity_;
  std::list<Key> lru_data_;
};

#endif  // DYNAMIC_CACHE_H_

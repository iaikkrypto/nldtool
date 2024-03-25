#ifndef CACHE_BASE_H_
#define CACHE_BASE_H_

#include <cstdint>
#include <cstdlib>
#include <cstring>

#include "managable_cache.h"

/*!
 * \brief Abstract base class for all caches, defining their interface
 */
template <class Key, class Data>
class CacheBase : public ManagableCache {
 public:
  struct Match {
    Key first;
    Data second;
    bool matched;
  };

  CacheBase() {}
  virtual ~CacheBase() {}
  virtual void Insert(const Key key, const Data value) = 0;
  virtual Match Find(const Key key) /*const*/ = 0;
  virtual void Clear() = 0;
  virtual int64_t GetSize() = 0;
  virtual void SetSize(int64_t size) = 0;
  virtual bool IsInitialized() = 0;
  virtual void SetInitialized() = 0;

  virtual void SetDumpFilepathAndLoadDump(const char* filepath) {
    if (this->filepath != nullptr) return;
    this->filepath = (char*)std::malloc(strlen(filepath) + 8);
    strcpy(this->filepath, "caches/");
    strcpy(this->filepath + 7, filepath);
    LoadDump();
    // printf("cache %lu got filename %s\n", reinterpret_cast<uintptr_t>(this),
    // filepath);
  }
};

#endif  // CACHE_BASE_H_

#ifndef CACHE_MANAGER_H_
#define CACHE_MANAGER_H_

#include <vector>

#include "managable_cache.h"

/*!
 * \brief A singleton class that manages all caches and can dump them all if
 * requested.
 */
class CacheManager {
  std::vector<ManagableCache*> caches;
  static CacheManager* instance;
  bool already_dumped;

  CacheManager() { already_dumped = false; }

  static CacheManager* getInstance() {
    if (instance == nullptr) instance = new CacheManager();
    return instance;
  }

 public:
  static void RegisterCache(ManagableCache* cache) {
    getInstance()->caches.push_back(cache);
  }

  static void DumpAllCaches(bool dump_again = false) {
    getInstance()->DumpAllCachesInternal(dump_again);
  }

  static void Destroy() {
    if (instance != nullptr) {
      getInstance()->caches.clear();
      delete instance;
      instance = nullptr;
    }
  }

 private:
  void DumpAllCachesInternal(bool dump_again) {
    if (already_dumped && !dump_again) return;
    already_dumped = true;
    CacheManager* instance = getInstance();
    for (int i = 0; i < instance->caches.size(); ++i)
      instance->caches[i]->SaveDump();
  }
};

#endif  // CACHE_MANAGER_H_

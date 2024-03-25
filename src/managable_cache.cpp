#include "managable_cache.h"

#include <cstdlib>

#include "cache_manager.h"

ManagableCache::ManagableCache() {
  filepath = nullptr;
  CacheManager::RegisterCache(this);
}

ManagableCache::~ManagableCache() {
  CacheManager::Destroy();
  if (filepath) free(filepath);
}

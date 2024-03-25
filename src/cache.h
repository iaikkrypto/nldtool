#ifndef CACHE_H_
#define CACHE_H_

#ifndef MAX_STATIC_CACHE_SIZE
#define MAX_STATIC_CACHE_SIZE 16
#endif

#include <type_traits>

#include "dynamic_cache.h"
#include "static_cache.h"

/*!
 * \brief A wrapper that automatically selects a static or dynamic cache based
 * on the size of the respective function
 */
template <class Key, class Data>
struct Cache {
  typedef typename std::conditional<(Key::NUMBITS <= MAX_STATIC_CACHE_SIZE),
                                    StaticCache<Key, Data>,
                                    DynamicCache<Key, Data>>::type result;
};

#endif  // CACHE_H_

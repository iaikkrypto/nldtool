#ifndef MANAGABLE_CACHE_H_
#define MANAGABLE_CACHE_H_

/*!
 * \brief The abstract base class, allowing to load and save cache dumps.
 */
class ManagableCache {
 public:
  ManagableCache();

  virtual ~ManagableCache();
  virtual bool LoadDump() = 0;
  virtual bool SaveDump() = 0;

 protected:
  char* filepath;
};

#endif  // MANAGABLE_CACHE_H_

#ifndef ADD2_H_
#define ADD2_H_

#include "crypto.h"

/*! \class add2
 *  \brief Implementation of Add2
 */
class Add2 : public Crypto {
 public:
  static void AddToOptions(cxxopts::Options& options);
  Add2(cxxopts::Options& options);
};

#endif  // ADD2_H_

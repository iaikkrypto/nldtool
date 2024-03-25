#ifndef XOR2_H_
#define XOR2_H_

#include "crypto.h"
#include "cxxopts.hpp"

/*! \class Xor2
 *  \brief Implementation of Xor2
 */
class Xor2 : public Crypto {
 public:
  static void AddToOptions(cxxopts::Options& options);
  Xor2(cxxopts::Options& options);
};

#endif  // XOR2_H_

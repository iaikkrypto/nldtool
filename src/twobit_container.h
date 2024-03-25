#ifndef TWOBIT_CONTAINER_H_
#define TWOBIT_CONTAINER_H_

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <memory>
#include <set>
#include <vector>

#include "bitmask.h"
#include "bitpos.h"
#include "condition.h"
#include "twobit_condition.h"

class Characteristic;
class Crypto;
typedef std::shared_ptr<Crypto> CryptoPtr;

/*!
 * \brief A container holding several \see TwobitConditions, providing some
 * helper functions.
 */
class TwobitContainer {
 public:
  TwobitContainer(CryptoPtr crypto);
  TwobitContainer(const TwobitContainer& s);
  TwobitContainer& operator=(const TwobitContainer& s);
  virtual ~TwobitContainer();

  void GenerateTwobitConditions(Characteristic& characteristic);
  void ClearTwobitConditions();
  const std::vector<TwobitCondition>& GetTwobitConditions() const;
  uint8_t GetTwobitDegree(Bitpos pos) const;
  void SetTwobitDegree(Bitpos pos, uint8_t degree);
  void IncTwobitDegree(Bitpos pos);
  std::vector<uint8_t> GetTwobitDegrees() const;
  Bitmask GetTwobitDegreeMask(int th = 1);
  int ComputeTwobitDegrees();
  int GetIndexFromBitpos(Bitpos pos);

 private:
  CryptoPtr crypto_;
  std::vector<TwobitCondition> twobit_conditions_;
  std::vector<uint8_t> twobit_degree_;
};

#endif  // TWOBIT_CONTAINER_H_

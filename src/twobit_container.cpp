#include "twobit_container.h"

#include "characteristic.h"
#include "crypto.h"
#include "logfile.h"
#include "text_io_format.h"

TwobitContainer::TwobitContainer(CryptoPtr crypto)
    : crypto_(crypto),
      twobit_conditions_(),
      twobit_degree_(crypto_->GetNumWords() * crypto_->GetWordSize(), 0) {}

TwobitContainer::TwobitContainer(const TwobitContainer& s)
    : crypto_(s.crypto_),
      twobit_conditions_(s.twobit_conditions_),
      twobit_degree_(s.twobit_degree_) {}

TwobitContainer::~TwobitContainer() {}

TwobitContainer& TwobitContainer::operator=(const TwobitContainer& s) {
  assert(crypto_.get() == s.crypto_.get());
  twobit_conditions_ = s.twobit_conditions_;
  twobit_degree_ = s.twobit_degree_;
  return *this;
}

void TwobitContainer::ClearTwobitConditions() {
  twobit_conditions_.clear();
  for (int pos = 0; pos < twobit_degree_.size(); ++pos) twobit_degree_[pos] = 0;
}

const std::vector<TwobitCondition>& TwobitContainer::GetTwobitConditions()
    const {
  return twobit_conditions_;
}

int TwobitContainer::ComputeTwobitDegrees() {
  int count = 0;
  TwobitCondition tmp;
  for (int i = 0; i < twobit_conditions_.size(); i++) {
    if (i == 0 || tmp < twobit_conditions_[i]) {
      IncTwobitDegree(twobit_conditions_[i].first_);
      IncTwobitDegree(twobit_conditions_[i].second_);
      count++;
    }
    tmp = twobit_conditions_[i];
  }
  return count;
}

int TwobitContainer::GetIndexFromBitpos(Bitpos pos) {
  return pos.GetWord() * crypto_->GetWordSize() + pos.GetBit();
}

void TwobitContainer::GenerateTwobitConditions(Characteristic& characteristic) {
  Bitmask& twobit_bitslice_update_mask = characteristic.GetTwobitUpdateMask();
  std::vector<Bitpos> twobit_bitslice_update_list =
      twobit_bitslice_update_mask.GetBitposList();

  //  std::cout << "twobit_update_list_ (" << twobit_bitslice_update_list.size()
  //  << ")" << std::endl; for (int i=0; i<twobit_bitslice_update_list.size();
  //  i++)
  //    std::cout << twobit_bitslice_update_list[i] << std::endl;
  //  std::cout << std::endl;

  ClearTwobitConditions();
  twobit_conditions_.reserve(1000);
  std::vector<TwobitCondition> temp;

  for (int i = 0; i < twobit_bitslice_update_list.size(); i++) {
    const Bitpos pos = twobit_bitslice_update_list.at(i);
    temp = crypto_->GetStep(pos.GetWord())
               .UpdateTwobitCondition(characteristic, pos);
    // if there is no twobit condition, clear bit in twobit update mask
    if (temp.size() == 0) twobit_bitslice_update_mask.ClearBit(pos);
    for (int j = 0; j < temp.size(); j++) {
      if (temp[j].condition_ == 4 || temp[j].condition_ == 8) {
        ConditionProxyPtr refA =
            characteristic.GetCrypto()->GetConditionProxy(temp[j].first_);
        ConditionProxyPtr refB =
            characteristic.GetCrypto()->GetConditionProxy(temp[j].second_);
        if ((refA->GetNumBits() == 1 &&
             (refA->GetCondition(characteristic) == BitCondition("-") ||
              refA->GetCondition(characteristic) == BitCondition("x"))) ||
            (refB->GetNumBits() == 1 &&
             (refB->GetCondition(characteristic) == BitCondition("-") ||
              refB->GetCondition(characteristic) == BitCondition("x"))))
          continue;
      }
      twobit_conditions_.push_back(temp[j]);
    }
  }
  std::sort(twobit_conditions_.begin(), twobit_conditions_.end());

  //  std::cout << "twobit_conditions_:" << std::endl;
  //  for (int i=0; i<twobit_conditions_.size(); i++)
  //    std::cout << twobit_conditions_[i] << std::endl;
  //  std::cout << std::endl;
}

Bitmask TwobitContainer::GetTwobitDegreeMask(const int th) {
  Bitmask mask(crypto_->GetWordSize());

  assert(th > 0);

  for (int i = 0; i < crypto_->GetNumWords() * crypto_->GetWordSize(); i++) {
    if (twobit_degree_[i] >= th) {
      mask.SetBit(i);
    }
  }

  return mask;
}

std::vector<uint8_t> TwobitContainer::GetTwobitDegrees() const {
  return twobit_degree_;
}

uint8_t TwobitContainer::GetTwobitDegree(Bitpos pos) const {
  return twobit_degree_[pos.GetWord() * crypto_->GetWordSize() + pos.GetBit()];
}

void TwobitContainer::SetTwobitDegree(Bitpos pos, uint8_t degree) {
  twobit_degree_[pos.GetWord() * crypto_->GetWordSize() + pos.GetBit()] =
      degree;
}

void TwobitContainer::IncTwobitDegree(Bitpos pos) {
  twobit_degree_[pos.GetWord() * crypto_->GetWordSize() + pos.GetBit()]++;
}

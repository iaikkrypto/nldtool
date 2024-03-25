#include "condition_word.h"

#include "characteristic.h"
#include "condition_proxy.h"
#include "container_condition_proxy.h"
#include "konstant_condition_proxy.h"

ConditionWord::ConditionWord()
    : container_index_(0),
      condition_mask_index_(0),
      num_bits_(0),
      name_(""),
      step_number_(0),
      word_type_(0),
      step_to_compute_probability_(0),
      only_container_condition_proxies_(false) {}

ConditionWord::ConditionWord(uint64_t constant, int word_size, std::string name)
    : container_index_(-1),
      condition_mask_index_(-1),
      num_bits_(1),
      name_(name),
      step_number_(-1),
      word_type_(3),
      step_to_compute_probability_(0),
      only_container_condition_proxies_(false) {
  for (int i = 0; i < word_size; ++i) {
    int condition = (nldtool::GetBit(constant, i) == 0) ? BitCondition("0")
                                                        : BitCondition("1");
    ConditionProxyPtr cp(new KonstantConditionProxy(1, condition));
    condition_proxies_.push_back(cp);
  }
}

ConditionWord::ConditionWord(int condition_mask_index, int container_index,
                             std::string name, int step_number, int word_type,
                             int word_size, int num_bits)
    : container_index_(container_index),
      condition_mask_index_(condition_mask_index),
      num_bits_(num_bits),
      name_(name),
      step_number_(step_number),
      word_type_(word_type),
      step_to_compute_probability_(0),
      only_container_condition_proxies_(false) {
  assert(1 <= num_bits_ && num_bits_ <= 3);
  only_container_condition_proxies_ = true;
  for (int i = 0; i < word_size; ++i) {
    ConditionProxyPtr cp(
        new ContainerConditionProxy(num_bits_, Bitpos(condition_mask_index_, i),
                                    Bitpos(container_index_, i)));
    condition_proxies_.push_back(cp);
  }
}

ConditionWord::ConditionWord(int condition_mask_index, std::string name,
                             int step_number, int word_type, int word_size,
                             int num_bits)
    : container_index_(-1),
      condition_mask_index_(condition_mask_index),
      num_bits_(num_bits),
      name_(name),
      step_number_(step_number),
      word_type_(word_type),
      step_to_compute_probability_(0),
      only_container_condition_proxies_(false) {
  for (int i = 0; i < word_size; ++i)
    condition_proxies_.push_back(ConditionProxyPtr());
}

ConditionWord::ConditionWord(int condition_mask_index_, int container_index,
                             std::string name, int step_number, int word_type,
                             int num_bits)
    : container_index_(container_index),
      condition_mask_index_(condition_mask_index_),
      num_bits_(num_bits),
      name_(name),
      step_number_(step_number),
      word_type_(word_type),
      step_to_compute_probability_(0),
      only_container_condition_proxies_(false) {}

ConditionWord::ConditionWord(const ConditionWord& sw)
    : container_index_(sw.container_index_),
      condition_mask_index_(sw.condition_mask_index_),
      num_bits_(sw.num_bits_),
      condition_proxies_(sw.condition_proxies_),
      name_(sw.name_),
      step_number_(sw.step_number_),
      word_type_(sw.word_type_),
      step_to_compute_probability_(sw.step_to_compute_probability_),
      only_container_condition_proxies_(sw.only_container_condition_proxies_) {}

ConditionWord::ConditionWord(const ConditionWord& sw, std::string name,
                             int step_number, int word_type, int num_bits)
    : container_index_(sw.container_index_),
      condition_mask_index_(sw.condition_mask_index_),
      num_bits_(num_bits),
      name_(name),
      step_number_(step_number),
      word_type_(word_type),
      step_to_compute_probability_(0),
      only_container_condition_proxies_(false) {}

ConditionWord& ConditionWord::operator=(const ConditionWord& sw) {
  name_ = sw.name_;
  step_number_ = sw.step_number_;
  word_type_ = sw.word_type_;
  container_index_ = sw.container_index_;
  condition_mask_index_ = sw.condition_mask_index_;
  num_bits_ = sw.num_bits_;
  condition_proxies_ = sw.condition_proxies_;
  only_container_condition_proxies_ = sw.only_container_condition_proxies_;
  return *this;
}

void ConditionWord::SetName(const std::string& name) { name_ = name; }

std::string ConditionWord::GetName() const { return name_; }

void ConditionWord::SetStepNumber(const int step_number) {
  step_number_ = step_number;
}

int ConditionWord::GetStepNumber() const { return step_number_; }

int ConditionWord::GetWordType() const { return word_type_; }

bool ConditionWord::IsMainWord() const { return word_type_ == 0; }

bool ConditionWord::IsSubWord() const { return word_type_ == 1; }

bool ConditionWord::IsCarryWord() const { return word_type_ == 2; }

bool ConditionWord::IsConstantWord() const { return word_type_ == 3; }

bool ConditionWord::IsHorizontalWord() const { return word_type_ == 4; }

int ConditionWord::GetWordSize() const { return condition_proxies_.size(); }

ConditionProxyPtr ConditionWord::GetConditionProxy(int bit) const {
  assert(0 <= bit && bit < condition_proxies_.size());
  assert(condition_proxies_[bit]);
  return condition_proxies_[bit];
}

void ConditionWord::SetConditionProxy(int bit, ConditionProxyPtr cp) {
  assert(0 <= bit && bit < condition_proxies_.size());
  assert(cp);
  condition_proxies_[bit] = cp;
  if (!dynamic_cast<ContainerConditionProxy*>(cp.get()))
    only_container_condition_proxies_ = false;
}

int ConditionWord::GetConditionMaskIndex() const {
  return condition_mask_index_;
}

bool ConditionWord::ContainsOnlyContainerConditionProxies() const {
  return only_container_condition_proxies_;
}

int ConditionWord::GetContainerIndex() const { return container_index_; }

int ConditionWord::GetNumBits() const { return num_bits_; }

std::ostream& operator<<(std::ostream& os, const ConditionWord& s) {
  os << "ConditionWordPtr(" << s.name_ << "," << s.step_number_
     << ")=" << s.condition_mask_index_ << " (" << s.word_type_ << ","
     << s.num_bits_ << ")";
  return os;
}

std::ostream& operator<<(std::ostream& os, const ConditionWord* s) {
  os << "ConditionWordPtr(" << s->name_ << "," << s->step_number_
     << ")=" << s->condition_mask_index_;
  return os;
}

void ConditionWord::Print(std::ostream& os) const {
  os << "ConditionWordPtr(" << name_ << "," << step_number_
     << ")=" << condition_mask_index_;
}

ConditionWordPtr ConditionWord::Rotr(int r) {
  ConditionWordPtr sw(new ConditionWord(*this));
  const int word_size = condition_proxies_.size();
  assert(-word_size < r && r < word_size);
  for (int i = 0; i < word_size; ++i)
    sw->SetConditionProxy(
        i, this->GetConditionProxy((i + r + word_size) % word_size));
  return sw;
}

ConditionWordPtr ConditionWord::Rotl(int r) { return Rotr(-r); }

ConditionWordPtr ConditionWord::Shr(int s) {
  if (s < 0) return Shl(-s);
  ConditionWordPtr sw(new ConditionWord(*this));
  int word_size = condition_proxies_.size();
  for (int i = 0; i < word_size; ++i)
    if ((0 <= i + s) && (i + s < word_size))
      sw->SetConditionProxy(i, this->GetConditionProxy(i + s));
    else
      sw->SetConditionProxy(
          i, ConditionProxyPtr(new KonstantConditionProxy(num_bits_)));
  return sw;
}

ConditionWordPtr ConditionWord::Shl(int s) {
  if (s < 0) return Shr(-s);
  ConditionWordPtr sw(new ConditionWord(*this));
  int word_size = condition_proxies_.size();
  for (int i = word_size - 1; i >= 0; --i)
    if ((0 <= i - s) && (i - s < word_size))
      sw->SetConditionProxy(i, this->GetConditionProxy(i - s));
    else
      sw->SetConditionProxy(
          i, ConditionProxyPtr(new KonstantConditionProxy(num_bits_)));
  return sw;
}

ConditionWordPtr ConditionWord::SetZero(int bit, int num, int start) {
  assert(num == 1 || num == 2);
  assert(0 <= start && start < num_bits_);
  ConditionWordPtr sw(new ConditionWord(*this));
  std::shared_ptr<VirtualConditionProxy> vcp(
      new VirtualConditionProxy(num_bits_));
  std::shared_ptr<LinkableConditionProxy> kcp(new KonstantConditionProxy(num));
  std::shared_ptr<LinkableConditionProxy> lcp =
      std::dynamic_pointer_cast<LinkableConditionProxy>(GetConditionProxy(bit));
  assert(vcp && lcp && kcp);
  vcp->AddConditionProxy(num, start, kcp, 0);
  if (num_bits_ - num > 0)
    vcp->AddConditionProxy(num_bits_ - num, (start + num) % num_bits_, lcp,
                           (start + num) % num_bits_);
  sw->SetConditionProxy(bit, vcp);
  return sw;
}

void ConditionWord::SetStepToComputeProbability(Step* step) {
  step_to_compute_probability_ = step;
}

Step* ConditionWord::GetStepToComputeProbability() {
  return step_to_compute_probability_;
}

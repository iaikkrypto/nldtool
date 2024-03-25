#include "horizontal_condition_word.h"

#include "condition_proxy.h"
#include "konstant_condition_proxy.h"
#include "linkable_condition_proxy.h"
#include "mask_condition_proxy.h"

HorizontalConditionWord::HorizontalConditionWord(int word_index,
                                                 int storage_index,
                                                 std::string name,
                                                 int step_number, int word_type,
                                                 int word_size, int num_bits)
    : ConditionWord(word_index, storage_index, name, step_number, word_type,
                    word_size, 2) {
  for (int i = 0; i < word_size; ++i) {
    std::shared_ptr<LinkableConditionProxy> prev =
        std::dynamic_pointer_cast<LinkableConditionProxy>(
            condition_proxies_[(i - 1 + word_size) % word_size]);
    std::shared_ptr<LinkableConditionProxy> next =
        std::dynamic_pointer_cast<LinkableConditionProxy>(
            condition_proxies_[(i + 1 + word_size) % word_size]);
    std::shared_ptr<MaskConditionProxy> vcp =
        std::dynamic_pointer_cast<MaskConditionProxy>(condition_proxies_[i]);
    assert(vcp && prev && next);
    vcp->AddConditionProxy(1, 0, prev, 1);
    vcp->AddConditionProxy(1, 1, next, 0);
  }
}

HorizontalConditionWord::HorizontalConditionWord(
    const HorizontalConditionWord& sw)
    : ConditionWord(sw) {}

HorizontalConditionWord::HorizontalConditionWord(uint64_t constant,
                                                 int word_size,
                                                 std::string name)
    : ConditionWord() {
  for (int i = 0; i < word_size; ++i) {
    BitCondition bit0 =
        nldtool::GetBit(constant, i) ? BitCondition("1") : BitCondition("0");
    BitCondition bit1 = nldtool::GetBit(constant, (i + 1) % word_size)
                            ? BitCondition("1")
                            : BitCondition("0");
    Condition<2> bc2(bit1, bit0);
    condition_proxies_.push_back(
        ConditionProxyPtr(new KonstantConditionProxy(2, bc2)));
  }
}

ConditionWordPtr HorizontalConditionWord::Rotr(int r) {
  if (r == 0) return ConditionWordPtr(this);
  ConditionWordPtr sw(new HorizontalConditionWord(*this));
  const int word_size = condition_proxies_.size();
  assert(-word_size < r && r < word_size);
  for (int i = 0; i < word_size; ++i)
    sw->SetConditionProxy(
        i, this->GetConditionProxy((i + r + word_size) % word_size));
  return sw;
}

ConditionWordPtr HorizontalConditionWord::Rotl(int r) { return Rotr(-r); }

ConditionWordPtr HorizontalConditionWord::Shr(int s) {
  if (s < 0) return Shl(-s);

  ConditionWordPtr sw = Rotr(s);
  std::shared_ptr<VirtualConditionProxy> vcp;
  std::shared_ptr<LinkableConditionProxy> lcp;
  std::shared_ptr<KonstantConditionProxy> kcp1(new KonstantConditionProxy(1));
  std::shared_ptr<KonstantConditionProxy> kcp2(new KonstantConditionProxy(2));
  const int word_size = condition_proxies_.size();

  vcp = std::make_shared<VirtualConditionProxy>(2);
  lcp = std::dynamic_pointer_cast<LinkableConditionProxy>(
      GetConditionProxy(word_size - 1));
  vcp->AddConditionProxy(1, 0, kcp1, 0);
  vcp->AddConditionProxy(1, 1, lcp, 1);
  sw->SetConditionProxy(word_size - 1, vcp);

  for (int i = word_size - 2; (i >= word_size - 1 - s + 1) && (i >= 0); --i)
    sw->SetConditionProxy(i, kcp2);

  vcp = std::make_shared<VirtualConditionProxy>(2);
  lcp = std::dynamic_pointer_cast<LinkableConditionProxy>(
      GetConditionProxy(word_size - 1 - s + 1));
  vcp->AddConditionProxy(1, 0, lcp, 0);
  vcp->AddConditionProxy(1, 1, kcp1, 0);
  sw->SetConditionProxy(word_size - 1 - s + 1, vcp);

  return sw;
}

ConditionWordPtr HorizontalConditionWord::Shl(int s) {
  if (s < 0) return Shr(-s);

  ConditionWordPtr sw = Rotl(s);
  std::shared_ptr<VirtualConditionProxy> vcp;
  std::shared_ptr<LinkableConditionProxy> lcp;
  std::shared_ptr<KonstantConditionProxy> kcp1(new KonstantConditionProxy(1));
  std::shared_ptr<KonstantConditionProxy> kcp2(new KonstantConditionProxy(2));
  const int word_size = condition_proxies_.size();

  vcp = std::make_shared<VirtualConditionProxy>(2);
  lcp = std::dynamic_pointer_cast<LinkableConditionProxy>(
      GetConditionProxy(s - 1));
  vcp->AddConditionProxy(1, 0, kcp1, 0);
  vcp->AddConditionProxy(1, 1, lcp, 1);
  sw->SetConditionProxy(s - 1, vcp);

  for (int i = 0; (i < s - 1) && (i < word_size); ++i)
    sw->SetConditionProxy(i, kcp2);

  vcp = std::make_shared<VirtualConditionProxy>(2);
  lcp = std::dynamic_pointer_cast<LinkableConditionProxy>(
      GetConditionProxy(word_size - 1));
  vcp->AddConditionProxy(1, 0, lcp, 0);
  vcp->AddConditionProxy(1, 1, kcp1, 0);
  sw->SetConditionProxy(word_size - 1, vcp);

  return sw;
}

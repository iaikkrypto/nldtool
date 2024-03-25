#include "crypto.h"

#include "characteristic.h"
#include "horizontal_condition_word.h"
#include "linkable_condition_proxy.h"
#include "mask_condition_proxy.h"

Crypto::~Crypto() {
  for (int i = 0; i < steps_.size(); i++) delete steps_[i];
  delete text_io_format_;
}

Crypto::Crypto(int word_size) {
  assert(word_size >= 1 && word_size <= 64);
  word_size_ = word_size;
  word_mask_ = nldtool::Mask(word_size);
  text_io_format_ = new TextIOFormat();
  num_words_[0] = 0;
  num_words_[1] = 0;
  num_words_[2] = 0;
  num_words_[3] = 0;
}

ConditionWordPtr Crypto::AddConditionWord(std::string name, int step_number,
                                          int row, int col, int type,
                                          int num_bits) {
  ConditionWordPtr condition_word;
  if (type == HORIZONTALWORD) {
    num_bits = 2;
    condition_word = ConditionWordPtr(
        new HorizontalConditionWord(words_.size(), num_words_[num_bits]++, name,
                                    step_number, type, word_size_, num_bits));
  } else
    condition_word = ConditionWordPtr(
        new ConditionWord(words_.size(), num_words_[num_bits]++, name,
                          step_number, type, word_size_, num_bits));

  auto success = word_handle_to_index_.insert(
      std::pair<WordHandle, int>(WordHandle(name, step_number), words_.size()));
  if (!success.second) {
    std::cerr << "Warning: duplicate name for ConditionWordPtr " << name
              << " in step " << step_number << "!" << std::endl;
  }
  words_.push_back(condition_word);
  if (row >= 0 && col >= 0) text_io_format_->SetStep(row, col, condition_word);
  return condition_word;
}

ConditionWordPtr Crypto::AddReferenceToConditionWord(
    ConditionWordPtr sw, std::string name, int step_number, int row, int col,
    int type, int num_bits, int start_bit) {
  // FIXME: do not generate ContainerConditionProxy
  ConditionWordPtr condition_word(new ConditionWord(
      words_.size(), name, step_number, type, word_size_, num_bits));
  const int word_size = sw->GetWordSize();
  for (int i = 0; i < word_size; ++i) {
    std::shared_ptr<MaskConditionProxy> smcp(
        new MaskConditionProxy(num_bits, Bitpos(words_.size(), i)));
    std::shared_ptr<VirtualConditionProxy> vcp(
        std::dynamic_pointer_cast<VirtualConditionProxy>(
            sw->GetConditionProxy(i)));
    if (vcp) smcp->AddConditionProxy(num_bits, start_bit, vcp);
    std::shared_ptr<LinkableConditionProxy> lcp(
        std::dynamic_pointer_cast<LinkableConditionProxy>(
            sw->GetConditionProxy(i)));
    if (lcp) smcp->AddConditionProxy(num_bits, 0, lcp, start_bit);
    condition_word->SetConditionProxy(i, smcp);
  }
  auto success = word_handle_to_index_.insert(
      std::pair<WordHandle, int>(WordHandle(name, step_number), words_.size()));
  if (!success.second) {
    std::cerr << "Warning: duplicate name for ConditionWordPtr " << name
              << " in step " << step_number << "!" << std::endl;
  }
  words_.push_back(condition_word);
  if (row >= 0 && col >= 0) text_io_format_->SetStep(row, col, condition_word);
  return condition_word;
}

Step* Crypto::Add(Step* step, Step::Priority priority) {
  if (step->IsCarryStep()) {
    const ConditionWordPtr last = step->GetParam(step->GetNumParams() - 1);
    ConditionWordPtr carry_word =
        AddConditionWord(std::string("C") + last->GetName(),
                         last->GetStepNumber(), -1, -1, CARRYWORD, 3);
    step->PopParam();
    step->AddParam(carry_word->Shl(1));
    step->AddParam(last);
    step->AddParam(carry_word);
  }
  step->Init(steps_.size(), priority);
  steps_.push_back(step);
  // SetupBitslicesToTouch(steps_.size() - 1);
  SetupOtherWords();
  return step;
}

void Crypto::SetupOtherWords() {
  ConditionWordPtr word;
  int w;
  for (int i = 0; i < steps_.back()->GetNumParams(); i++) {
    bool notStored = true;
    word = steps_.back()->GetParam(i);
    for (w = 0; w < words_.size(); w++)
      if (words_[w] == word) notStored = false;
    for (w = 0; w < other_words_.size(); w++)
      if (other_words_[w] == word) notStored = false;
    if (notStored) other_words_.push_back(word);
  }
}

bool Crypto::Callback(std::string function, Characteristic& characteristic,
                      std::mt19937& rng, Logfile& logfile) {
  if (function.empty())
    return true;
  else if (function.compare("found") == 0) {
    logfile << "found new characteristic" << std::endl;
    characteristic.WriteCharacteristic(logfile);
    return true;
  }
  // this should never happen!
  assert(false);
  return false;
}

Bitmask Crypto::GetWordMaskMain() const {
  Bitmask condition_mask(GetWordSize());
  for (int i = 0; i < GetNumWords(); ++i)
    if (words_[i]->GetWordType() == MAINWORD) condition_mask.SetWord(i);
  return condition_mask;
}

int Crypto::GetConditionWordIndex(const std::string& name, int step) const {
  return word_handle_to_index_.find(WordHandle(name, step))->second;
}

Bitmask Crypto::GetConditionWordMaskRegex(const std::regex& name_regex) const {
  Bitmask condition_mask(GetWordSize());
  for (int i = 0; i < GetNumWords(); ++i) {
    if (std::regex_match(words_[i]->GetName(), name_regex))
      condition_mask.SetWord(i);
  }
  return condition_mask;
}

Bitmask Crypto::GetConditionWordIntervalMaskRegex(const std::regex& name_regex,
                                                  int start_step,
                                                  int end_step) const {
  Bitmask condition_mask(GetWordSize());
  for (int i = 0; i < GetNumWords(); ++i)
    if (std::regex_match(words_[i]->GetName(), name_regex) &&
        words_[i]->GetStepNumber() >= start_step &&
        words_[i]->GetStepNumber() <= end_step)
      condition_mask.SetWord(i);
  return condition_mask;
}

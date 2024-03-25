#include "characteristic.h"

#include <cassert>
#include <iostream>

#include "search.h"
#include "text_io_format.h"

Characteristic::Characteristic(CryptoPtr crypto)
    : crypto_(crypto),
      bit_conditions_(crypto_->GetNumWords(1), crypto_->GetWordSize()),
      bit_conditions2_(crypto_->GetNumWords(2), crypto_->GetWordSize()),
      bit_conditions3_(crypto_->GetNumWords(3), crypto_->GetWordSize()),
      step_data_(crypto_->GetNumSteps(), (StepData*)0),
      twobit_container_(crypto_),
      step_update_list_(),
      condition_update_list_(),
      twobit_bitslice_update_mask_(crypto_->GetWordSize()) {
  for (int step = 0; step < crypto_->GetNumSteps(); ++step) {
    step_data_[step] = crypto_->GetStep(step).CreateStepData();
  }
}

Characteristic::Characteristic(const Characteristic& s)
    : crypto_(s.crypto_),
      bit_conditions_(s.bit_conditions_),
      bit_conditions2_(s.bit_conditions2_),
      bit_conditions3_(s.bit_conditions3_),
      step_data_(s.step_data_),
      twobit_container_(s.twobit_container_),
      step_update_list_(s.step_update_list_),
      condition_update_list_(s.condition_update_list_),
      twobit_bitslice_update_mask_(s.twobit_bitslice_update_mask_) {
  for (int step = 0; step < crypto_->GetNumSteps(); ++step) {
    if (s.step_data_[step] != nullptr)
      step_data_[step] = (s.step_data_[step])->Clone();
    else
      step_data_[step] = nullptr;
  }
}

Characteristic::~Characteristic() {
  for (int step = 0; step < crypto_->GetNumSteps(); ++step) {
    delete step_data_[step];
  }
}

Characteristic& Characteristic::operator=(const Characteristic& s) {
  assert(crypto_.get() == s.crypto_.get());
  bit_conditions_ = s.bit_conditions_;
  bit_conditions2_ = s.bit_conditions2_;
  bit_conditions3_ = s.bit_conditions3_;
  twobit_container_ = s.twobit_container_;
  step_update_list_ = s.step_update_list_;
  condition_update_list_ = s.condition_update_list_;
  twobit_bitslice_update_mask_ = s.twobit_bitslice_update_mask_;
  for (int step = 0; step < crypto_->GetNumSteps(); ++step) {
    if (s.step_data_[step] != 0) {
      (s.step_data_[step])->Copy(&(step_data_[step]));
    }
  }
  return *this;
}

Characteristic::Characteristic(Characteristic&& s)
    : crypto_(s.crypto_),
      bit_conditions_(s.bit_conditions_),
      bit_conditions2_(s.bit_conditions2_),
      bit_conditions3_(s.bit_conditions3_),
      step_data_(s.step_data_),
      twobit_container_(s.twobit_container_),
      step_update_list_(s.step_update_list_),
      condition_update_list_(s.condition_update_list_),
      twobit_bitslice_update_mask_(s.twobit_bitslice_update_mask_) {
  // do not copy the step data
  // step_data_.reserve(crypto_->GetNumSteps());
  for (int step = 0; step < s.crypto_->GetNumSteps(); ++step) {
    // step_data_[step] = s.step_data_[step];
    s.step_data_[step] = nullptr;
  }
}

Characteristic& Characteristic::operator=(Characteristic&& s) {
  crypto_ = s.crypto_;

  for (int step = 0; step < crypto_->GetNumSteps(); ++step) {
    delete step_data_[step];
  }

  bit_conditions_ = s.bit_conditions_;
  bit_conditions2_ = s.bit_conditions2_;
  bit_conditions3_ = s.bit_conditions3_;
  twobit_container_ = s.twobit_container_;
  step_update_list_ = std::move(s.step_update_list_);
  condition_update_list_ = std::move(s.condition_update_list_);
  twobit_bitslice_update_mask_ = s.twobit_bitslice_update_mask_;
  // step_data_ = std::move(s.step_data_);
  step_data_.reserve(crypto_->GetNumSteps());
  for (int step = 0; step < s.crypto_->GetNumSteps(); ++step) {
    step_data_[step] = s.step_data_[step];
    s.step_data_[step] = nullptr;
  }
  return *this;
}

void Characteristic::ShallowCopy(const Characteristic& s) {
  // equals operator= except it shares the linear matrix instead of copying
  assert(crypto_.get() == s.crypto_.get());
  bit_conditions_ = s.bit_conditions_;
  bit_conditions2_ = s.bit_conditions2_;
  bit_conditions3_ = s.bit_conditions3_;
  twobit_container_ = s.twobit_container_;
  step_update_list_ = s.step_update_list_;
  condition_update_list_ = s.condition_update_list_;
  twobit_bitslice_update_mask_ = s.twobit_bitslice_update_mask_;
  for (int step = 0; step < crypto_->GetNumSteps(); ++step) {
    if (s.step_data_[step] != 0) {
      step_data_[step]->ShallowCopy(*s.step_data_[step]);
    }
  }
}

void Characteristic::Undo() {
  for (int step = 0; step < crypto_->GetNumSteps(); ++step)
    if (step_data_[step] != 0) step_data_[step]->Undo();
}

void Characteristic::UndoAndRestore() {
  for (int step = 0; step < crypto_->GetNumSteps(); ++step)
    if (step_data_[step] != 0) step_data_[step]->UndoAndRestore();
}

StepData& Characteristic::GetStepData(int step) { return *(step_data_[step]); }

const StepData& Characteristic::GetStepData(int step) const {
  return *(step_data_[step]);
}

Bitmask& Characteristic::GetTwobitUpdateMask() {
  return twobit_bitslice_update_mask_;
}

TwobitContainer& Characteristic::GetTwobitConditions() {
  return twobit_container_;
}

const TwobitContainer& Characteristic::GetConstTwobitConditions() const {
  return twobit_container_;
}

void Characteristic::GenerateTwobitConditions() {
  twobit_container_.GenerateTwobitConditions(*this);
}

BitCondition Characteristic::GetBitCondition(Bitpos mask_pos) const {
  return BitCondition(
      crypto_->GetConditionProxy(mask_pos)->GetCondition(*this));
}

bool Characteristic::SetBitCondition(Bitpos mask_pos, BitCondition value) {
  return crypto_->GetConditionProxy(mask_pos)->SetCondition(*this, value);
}

CryptoPtr Characteristic::GetCrypto() const { return crypto_; }

void Characteristic::SetBitConditionWord(int condition_word, BitCondition value,
                                         uint64_t bit_mask) {
  assert(0 <= condition_word && condition_word < crypto_->GetNumWords());
  for (int condition_bit = 0; condition_bit < crypto_->GetWordSize();
       ++condition_bit, bit_mask >>= 1)
    if (bit_mask & 1)
      SetBitCondition(Bitpos(condition_word, condition_bit), value);
}

void Characteristic::SetWord(const std::string& name, int step, uint64_t a,
                             uint64_t b) {
  int word = crypto_->GetConditionWordIndex(name, step);
  for (int bit = 0; bit < crypto_->GetWordSize(); ++bit) {
    const ConditionProxyPtr cp = crypto_->GetConditionProxy(Bitpos(word, bit));
    const int cond = (nldtool::GetBit(b, bit) << 1) | nldtool::GetBit(a, bit);
    cp->SetCondition(*this, 1 << cond);
  }
}

bool Characteristic::GetWord(const std::string& name, int step, uint64_t& a,
                             uint64_t& b) {
  int word = crypto_->GetConditionWordIndex(name, step);
  for (int bit = 0; bit < crypto_->GetWordSize(); ++bit) {
    const ConditionProxyPtr cp = crypto_->GetConditionProxy(Bitpos(word, bit));
    const uint64_t cond = cp->GetCondition(*this);
    switch (cond) {
      case 1:  // '0'
        nldtool::SetBit(a, bit, 0);
        nldtool::SetBit(b, bit, 0);
        break;
      case 2:  // 'u'
        nldtool::SetBit(a, bit, 1);
        nldtool::SetBit(b, bit, 0);
        break;
      case 4:  // 'n'
        nldtool::SetBit(a, bit, 0);
        nldtool::SetBit(b, bit, 1);
        break;
      case 8:  // '1'
        nldtool::SetBit(a, bit, 1);
        nldtool::SetBit(b, bit, 1);
        break;
      default:
        return false;
    }
  }
  return true;
}

void Characteristic::SetBit(const std::string& name, int step, int bit, bool a,
                            bool b) {
  int word = crypto_->GetConditionWordIndex(name, step);
  const ConditionProxyPtr cp = crypto_->GetConditionProxy(Bitpos(word, bit));
  const int cond = (((uint64_t)b) << 1) | ((uint64_t)a);
  cp->SetCondition(*this, 1 << cond);
}

Bitmask Characteristic::GetBitConditionMask(Bitmask mask,
                                            BitCondition value) const {
  return GetConditionMask(mask,
                          [value](BitCondition bc) { return bc == value; });
}

Bitmask Characteristic::GetBitConditionMask(BitCondition value) const {
  return GetConditionMask([value](BitCondition bc) { return bc == value; });
}

uint64_t Characteristic::GetConditionWordMask(
    const std::function<bool(BitCondition)>& f, int i) const {
  const ConditionWordPtr condition_word = crypto_->GetConditionWord(i);
  if (condition_word->GetNumBits() != 1) return 0;
  uint64_t bitmask = 0;
  if (condition_word->ContainsOnlyContainerConditionProxies()) {
    int word_index = condition_word->GetContainerIndex();
    bitmask = bit_conditions_.GetConditionMask(word_index, f);
  } else {
    for (int j = 0; j < crypto_->GetWordSize(); ++j) {
      const bool b = f(GetBitCondition(Bitpos(i, j)));
      bitmask |= ((uint64_t)b) << j;
    }
  }
  return bitmask;
}

Bitmask Characteristic::GetConditionMask(
    Bitmask mask, const std::function<bool(BitCondition)>& f) const {
  for (auto x = mask.begin(); x != mask.end(); ++x)
    x = mask.ClearBits(x, ~GetConditionWordMask(f, x->word));
  return mask;
}

Bitmask Characteristic::GetConditionMask(
    const std::function<bool(BitCondition)>& f) const {
  Bitmask mask(crypto_->GetWordSize());
  for (auto x = mask.begin(); x != mask.end(); ++x)
    x = mask.SetBits(x, GetConditionWordMask(f, x->word));
  return mask;
}

std::vector<Bitpos> Characteristic::GetBitConditionList(
    int word, BitCondition value) const {
  const ConditionWordPtr condition_word = crypto_->GetConditionWord(word);
  if (condition_word->GetNumBits() != 1) return std::vector<Bitpos>();
  std::vector<Bitpos> list;
  list.reserve(condition_word->GetWordSize());
  for (int j = 0; j < condition_word->GetWordSize(); ++j)
    if (GetBitCondition(Bitpos(word, j)) == value)
      list.push_back(Bitpos(word, j));
  return list;
}

bool Characteristic::Complete(const std::function<bool(BitCondition)>& f) {
  return Complete(f, Bitmask(crypto_->GetWordSize()));
}

bool Characteristic::Complete(const std::function<bool(BitCondition)>& f,
                              const Bitmask& mask) {
  Characteristic tmp = *this;
  bool changed, results[2];
  do {
    changed = false;
    Bitmask todo = GetConditionMask(mask, f);
    while (!todo.Empty()) {
      const Bitpos p = todo.First();
      const uint8_t c = GetBitCondition(p);
      // try both choices:
      for (int i = 0; i < 2; ++i) {
        tmp.ShallowCopy(*this);
        tmp.SetBitCondition(p, BitCondition(Search::CHOICES[c].bc[i]));
        results[i] = tmp.Update(true);
        todo = tmp.GetConditionMask(todo, f);
        tmp.Undo();
      }
      // continue, return or redo:
      if (results[0] && results[1]) continue;
      if (!results[0] && !results[1]) {
        return false;
      }
      changed = true;
      SetBitCondition(p, BitCondition(Search::CHOICES[c].bc[results[1]]));
      Update();  // should never fail!
      break;
    }
  } while (changed);
  return true;
}

bool Characteristic::Update(bool backtrack, int32_t priority) {
  while (!step_update_list_.empty()) {
    const StepUpdate pos = *step_update_list_.begin();
    if (pos.priority > priority) {
      step_update_list_.erase(pos);
      continue;
    }
    if (!crypto_->GetStep(pos.step).Update(*this, pos.bit, backtrack)) {
      step_update_list_.erase(pos);
      return false;
    }
    step_update_list_.erase(pos);
    // FIXME: improve performance of LinearConditions::PropagateConditions and
    // remove the following hack
    if (step_update_list_.empty() ||
        (*step_update_list_.begin()).step != pos.step)
      if (!crypto_->GetStep(pos.step).PropagateConditions(*this)) {
        return false;
      }
  }
  return true;
}

void Characteristic::Touch(const ConditionProxy& cp) {
  for (auto pos : cp.GetStepsToUpdate()) {
    step_update_list_.insert(pos);
    if (crypto_->GetStep(pos.step).GetName().compare("LinearStep") != 0)
      twobit_bitslice_update_mask_.SetBit(Bitpos(pos.step, pos.bit));
  }
}

bool Characteristic::UpdateAll() {
  for (int word = 0; word < crypto_->GetNumWords(); word++)
    for (int bit = 0; bit < crypto_->GetWordSize(); bit++)
      Touch(*(crypto_->GetConditionProxy(Bitpos(word, bit)).get()));
  return Update();
}

int Characteristic::CheckCharacteristic(Logfile& logfile, int check_level,
                                        bool print_characteristic) {
  if (print_characteristic) {
    logfile << "Info: input characteristic:" << std::endl;
    WriteCharacteristic(logfile);
  }

  if (check_level <= 0) return 0;

  if (print_characteristic) logfile << "Info: update:" << std::endl;
  if (!UpdateAll()) {
    if (print_characteristic) WriteCharacteristic(logfile);
    logfile << "Info: update all... failed!" << std::endl;
    return 1;
  } else {
    if (print_characteristic) WriteCharacteristic(logfile);
    logfile << "Info: update all... ok!" << std::endl;
  }

  if (check_level <= 1) return 0;

  if (print_characteristic)
    logfile << std::endl << "Info: computing 2-bit conditions..." << std::endl;
  GenerateTwobitConditions();
  twobit_container_.ComputeTwobitDegrees();
  Bitmask mask = twobit_container_.GetTwobitDegreeMask(1);
  if (!Complete(
          [](BitCondition bc) {
            return bc == BitCondition("-") || bc == BitCondition("x");
          },
          mask)) {
    if (print_characteristic) WriteCharacteristic(logfile);
    logfile << "Info: complete on all 2-bit conditions... failed!" << std::endl;
    return 2;
  } else {
    GenerateTwobitConditions();
    twobit_container_.ComputeTwobitDegrees();
    if (print_characteristic) WriteCharacteristic(logfile);
    logfile << "Info: complete on all 2-bit conditions... ok!" << std::endl;
  }

  if (check_level <= 2) return 0;

  if (print_characteristic)
    logfile << "Info: complete on all -/x bits:" << std::endl;
  if (!Complete([](BitCondition bc) {
        return bc == BitCondition("-") || bc == BitCondition("x");
      })) {
    if (print_characteristic) WriteCharacteristic(logfile);
    logfile << "Info: complete on all -/x bits... failed!" << std::endl;
    return 3;
  } else {
    GenerateTwobitConditions();
    twobit_container_.ComputeTwobitDegrees();
    if (print_characteristic) WriteCharacteristic(logfile);
    logfile << "Info: complete on all -/x bits... ok!" << std::endl;
  }

  return 0;
}

float Characteristic::GetProbability(int word_index) const {
  Step* step =
      crypto_->GetConditionWord(word_index)->GetStepToComputeProbability();
  if (step)
    return step->GetProbability(*this);
  else
    return 0.0;
}

float Characteristic::GetProbabilitySum(int step_number) const {
  float prob = 0.0;
  for (int i = 0; i < crypto_->GetNumWords(); i++) {
    Step* step = crypto_->GetConditionWord(i)->GetStepToComputeProbability();
    if (step && crypto_->GetConditionWord(i)->GetStepNumber() == step_number)
      prob += step->GetProbability(*this);
  }
  return prob;
}

float Characteristic::GetProbabilitySum() const {
  float prob = 0.0;
  for (int i = 0; i < crypto_->GetNumWords(); i++) {
    Step* step = crypto_->GetConditionWord(i)->GetStepToComputeProbability();
    if (step) prob += step->GetProbability(*this);
  }
  return prob;
}

void Characteristic::WriteCharacteristic(std::string file) {
  if (file == "") {
    WriteCharacteristic(std::cout);
  } else {
    std::fstream stream;
    stream.open(file.c_str(), std::ios::out | std::ios::app);
    if (stream.fail()) {
      std::cout << "error: writing file '" << file << "'" << std::endl;
      return;
    }
    WriteCharacteristic(stream);
    stream.close();
  }
}

bool Characteristic::ReadCharacteristic(std::istream& fs,
                                        bool only_main_steps) {
  return crypto_->GetTextIOFormat()->ReadCharacteristic(fs, *this,
                                                        only_main_steps);
}

void Characteristic::WriteCharacteristic(std::ostream& fs) {
  crypto_->GetTextIOFormat()->WriteCharacteristic(fs, *this);
  //  for (int i=0; i<step_data_.size(); i++)
  //    step_data_[i]->Print();
}

void Characteristic::WriteCharacteristic(Logfile& logfile) {
  WriteCharacteristic(logfile.getStream());
}

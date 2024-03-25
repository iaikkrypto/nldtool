#include "search.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>

#include "cache.h"
#include "cache_manager.h"
#include "change.h"
#include "crypto.h"
#include "logfile.h"
#include "text_io_format.h"
#include "xml_config.h"

//#define DEBUG_CONFIGSEARCH

constexpr Search::Choice Search::CHOICES[16];

Search::Search(XmlConfig& config, Characteristic& characteristic,
               Logfile& logfile, int stack_fullstate_frequency, int64_t seed)
    : twobit_conditions_generated_(false),
      complete_check_(false),
      config_(config),
      start_characteristic_(characteristic),
      tmp_characteristic_(characteristic),
      characteristic_(characteristic),
      initial_free_bits_mask_(characteristic_.GetConditionMask(
          characteristic_.GetCrypto()->GetWordMaskMain(),
          [](BitCondition bc) {
            return ((bc & -bc) ^ bc) && (bc ^ BitCondition("-"));
          })),
      search_stack_(stack_fullstate_frequency),
      logfile_(logfile),
      seed_(seed) {
  if (seed == -1) {
    std::random_device device;
    seed_ = static_cast<uint32_t>(device());
  } else {
    seed_ = static_cast<uint32_t>(seed);
  }
  rng_.seed(static_cast<unsigned long>(seed_));
  current_status_.seed = seed_;
  seed_time_ = print_time_ = print_characteristic_time_ = time(0);
  seed_ = static_cast<uint32_t>(rng_());
  current_status_.restarts = 0;
  current_status_.found = 0;
  current_status_.absminfree = initial_free_bits_mask_.GetNumBitsSet();
  current_status_.Init();
}

Search::~Search() {}

void Search::PrintInfo(bool print_characteristic) {
  logfile_ << current_status_ << std::endl;
  if (print_characteristic) {
    characteristic_.GenerateTwobitConditions();
    characteristic_.GetTwobitConditions().ComputeTwobitDegrees();

    if (config_.GetSearchConfig().print_whole_characteristic == 1) {
      characteristic_.GetCrypto()->GetTextIOFormat()->SetWriteAll(true);
      config_.GetSearchConfig().print_whole_characteristic = 0;
    }
    characteristic_.WriteCharacteristic(logfile_);
  }
}

void Search::SetDumpTime(int dump_time) {
  current_status_.dump_time =
      dump_time * 3600;  // conversion from hours to seconds
}

void Search::SetDumpCount(int dump_count) {
  current_status_.dump_count = dump_count;
}

void Search::CheckforDump() {
  if (current_status_.dump_time <= 0 && current_status_.dump_count <= 0)
    return;  // not active
  if ((current_status_.dump_time > 0 &&
       time(0) - current_status_.start_time >= current_status_.dump_time) ||
      (current_status_.dump_count > 0 &&
       current_status_.restarts >= current_status_.dump_count)) {
    printf(
        "\ndump condition (time: %d, restarts: %d) has been reached -> start "
        "dumping\n",
        (int)time(0) - current_status_.start_time, current_status_.restarts);
    CacheManager::DumpAllCaches();
    printf("finished dumping -> exit program\n\n");
    exit(0);
  }
}

void Search::Restart() {
  int current_time = time(0);
  if (config_.GetSearchConfig().print_info != -1 &&
      current_time - print_time_ >= config_.GetSearchConfig().print_info &&
      current_status_.global_iterations != 0) {
    print_time_ = current_time;
    PrintInfo();
  }
  search_stack_.clear();
  characteristic_ = start_characteristic_;
  critical_bits_.clear();
  critical_bit_index_ = 0;
  guess_critical_bits_ = false;
  complete_check_ = false;
  current_status_.Init();
  if (config_.GetSearchConfig().reseed != -1 &&
      (config_.GetSearchConfig().reseed == 0 ||
       current_time - seed_time_ >= config_.GetSearchConfig().reseed)) {
    seed_time_ = current_time;
    rng_.seed(static_cast<unsigned long>(seed_));
    real_rng_.seed(static_cast<unsigned long>(seed_));
    current_status_.seed = seed_;
    seed_ = static_cast<uint32_t>(rng_());
  }
  switched_back_or_stack_empty_ = false;
  current_status_.phase = 0;
  current_status_.restarts++;
  current_status_.remaining_credits = credits_;
  characteristic_.GetCrypto()->Callback(config_.GetSearchConfig().callback,
                                        characteristic_, rng_, logfile_);
  search_stack_.push_back(characteristic_, 0);
}

Bitmask Search::ComputeTwobitMask(int threshold) {
  characteristic_.GenerateTwobitConditions();
  characteristic_.GetTwobitConditions().ComputeTwobitDegrees();
  Bitmask twobit_mask =
      characteristic_.GetTwobitConditions().GetTwobitDegreeMask(threshold);
  return twobit_mask;
}

bool Search::CompleteCheck(Bitmask mask) {
  bool result = true;
  std::set<Bitpos> list;
  while (!switched_back_or_stack_empty_ &&
         current_status_.remaining_credits >= 0 &&
         !characteristic_.Complete(
             [](BitCondition bc) {
               return bc == BitCondition("-") || bc == BitCondition("x");
             },
             mask)) {
    BackTrack();
    guess_critical_bits_ = false;
    result = false;
  }
  for (auto x : list) AddCriticalBit({GetLastPhase().backtrack_guesses, x});
  return result;
}

void Search::UpdateStatus() {
  current_status_.iterations++;
  current_status_.global_iterations++;
  current_status_.stack_size = search_stack_.size();
  current_status_.max_stack_size =
      std::max(current_status_.max_stack_size, current_status_.stack_size);
  if (UpdateMinfree() &&
      current_status_.free_bits < config_.GetSearchConfig().print_minfree)
    PrintInfo(true);
  CheckforDump();
}

bool Search::UpdateMinfree() {
  current_status_.free_bits =
      characteristic_
          .GetConditionMask(initial_free_bits_mask_,
                            [](BitCondition bc) {
                              return ((bc & -bc) ^ bc) &&
                                     (bc ^ BitCondition("-"));
                            })
          .GetNumBitsSet();
  bool result = current_status_.free_bits < current_status_.absminfree;
  current_status_.minfree =
      std::min(current_status_.minfree, current_status_.free_bits);
  current_status_.absminfree =
      std::min(current_status_.absminfree, current_status_.minfree);
  return result;
}

bool Search::IsGuessable(const Config::Setting& setting, BitCondition bc) {
  return std::any_of(setting.guesses.begin(), setting.guesses.end(),
                     [bc](Config::Guess guess) { return bc == guess.bc; });
}

bool Search::GenerateSearchMasks(const std::vector<Config::Setting>& settings) {
  twobit_conditions_generated_ = false;
  searchmasks_.clear();
  searchmasks_.reserve(settings.size());
  for (const Config::Setting& setting : settings)
    searchmasks_.emplace_back(characteristic_.GetConditionMask(
        GenerateWordMask(setting),
        std::bind(IsGuessable, setting, std::placeholders::_1)));
  bool result = std::any_of(searchmasks_.begin(), searchmasks_.end(),
                            [](const Bitmask& mask) { return !mask.Empty(); });
  return result;
}

Bitmask Search::GenerateWordMask(const Config::Setting& setting) {
  int num_words = characteristic_.GetCrypto()->GetNumWords();
  int word_size = characteristic_.GetCrypto()->GetWordSize();
  Bitmask mask(word_size);
  if (setting.masks.size() == 0) {
    mask.SetAll(num_words);
  } else {
    Bitmask tmp(word_size);
    for (int i = 0; i < setting.masks.size(); ++i) {
      const Config::Mask word = setting.masks[i];
      tmp = word.bitmask;
      if (word.twobit_threshold > 0) {
        if (twobit_conditions_generated_ == false) {
          characteristic_.GenerateTwobitConditions();
          characteristic_.GetTwobitConditions().ComputeTwobitDegrees();
          twobit_conditions_generated_ = true;
        }
        tmp &= characteristic_.GetTwobitConditions().GetTwobitDegreeMask(
            word.twobit_threshold);
      }
      mask |= tmp;
    }
  }
  return mask;
}

const Search::Config::Phase& Search::GetCurrentPhase() {
  return config_.GetSearchPhase(current_status_.phase);
}

const Search::Config::Phase& Search::GetLastPhase() {
  return config_.GetSearchPhase(config_.GetSearchConfig().phases.size() - 1);
}

const Search::Config::Setting& Search::GetSetting(int index) {
  return GetCurrentPhase().settings[index];
}

int Search::ChooseSetting() {
  // returns the index of a setting chosen according to the specified
  // distribution that still has bits left to guess, or -1 if none exists
  int num_settings = GetCurrentPhase().settings.size();
  assert(searchmasks_.size() == num_settings);
  std::vector<int> indices;
  std::vector<double> weights;
  indices.reserve(num_settings);
  weights.reserve(num_settings);
  for (int i = 0; i < num_settings; ++i)
    if (!searchmasks_[i].Empty()) {
      indices.emplace_back(i);
      weights.emplace_back(GetSetting(i).probability);
    }
  if (indices.empty()) return -1;
  std::discrete_distribution<int> distribution(weights.begin(), weights.end());
  int index = indices[distribution(rng_)];
  return index;
}

bool Search::GuessBit(Config::Guess guess, Bitpos pos) {
  std::uniform_real_distribution<double> real_rand(0.0, 1.0);
  bool pushstack =
      guess.choice_probability > 0.0 && guess.choice_probability < 1.0;
  int index = guess.choice_probability >= real_rand(real_rng_) ? 0 : 1;

  // copy Conditions<N>, but let both chars point to the *same* shared matrix
  tmp_characteristic_.ShallowCopy(characteristic_);

  // first choice
  BitCondition cond = BitCondition(CHOICES[guess.bc].bc[index]);
  characteristic_.SetBitCondition(pos, cond);
  bool first = characteristic_.Update(true);

  if (first) {
    search_stack_.add_guess(characteristic_, pos, cond,
                            pushstack
                                ? BitCondition(CHOICES[guess.bc].bc[1 - index])
                                : BitCondition("#"));
    return true;
  }

  // return to matrix of tmp_characteristic_
  characteristic_.Undo();

  // second choice
  cond = BitCondition(CHOICES[guess.bc].bc[1 - index]);
  tmp_characteristic_.SetBitCondition(pos, cond);
  bool second = tmp_characteristic_.Update(true);

  if (!second) return false;

  characteristic_.ShallowCopy(tmp_characteristic_);
  search_stack_.add_guess(characteristic_, pos, cond);

  return true;
}

bool Search::Guess(GuessTask pos) {
  const BitCondition bc = characteristic_.GetBitCondition(pos.pos);
  const std::vector<Config::Guess>& guesses = pos.guesses;
  auto guess = std::find_if(guesses.begin(), guesses.end(),
                            [bc](Config::Guess g) { return g.bc == bc; });
  while (guess != guesses.end()) {
    if (!GuessBit(*guess, pos.pos)) return false;
    const BitCondition bc = characteristic_.GetBitCondition(pos.pos);
    guess = std::find_if(guesses.begin(), guesses.end(),
                         [bc](Config::Guess g) { return g.bc == bc; });
  }
  return true;
}

void Search::AddCriticalBit(GuessTask bit) {
  // TODO: back track limit is just hard coded for now
  static const int btlimit = 10;
  critical_bits_.push_front(bit);
  if (critical_bits_.size() > btlimit) critical_bits_.pop_back();
}

void Search::Start() {
  int complete_limit = 20;

  current_status_.phase = 0;
  current_status_.start_time = time(0);
  current_status_.global_iterations = 0l;
  credits_ = config_.GetSearchConfig().credits;
  Restart();

  logfile_ << "Info: Search started..." << std::endl;
  while (1) {
    //! print info if necessary and check for termination conditions
    UpdateStatus();

    int current_time = time(0);
    if (config_.GetSearchConfig().print_info != -1 &&
        current_time - print_time_ >=
            config_.GetSearchConfig().print_info * 2) {
      print_time_ = current_time;
      PrintInfo();
    }
    if (config_.GetSearchConfig().print_characteristic != -1 &&
        current_time - print_characteristic_time_ >=
            config_.GetSearchConfig().print_characteristic) {
      print_characteristic_time_ = current_time;
      PrintInfo(true);
    }
    if (config_.GetSearchConfig().end_time != -1 &&
        current_time - current_status_.start_time >
            config_.GetSearchConfig().end_time)
      exit(1);

    //! handle restart conditions
    if ((search_stack_.empty() &&
         credits_ < config_.GetSearchConfig().credits) ||
        current_status_.remaining_credits <= 0) {
      Restart();
      continue;
    }

    //! if we saved any critical bit positions, guess them first
    if (guess_critical_bits_) {
      assert(0 <= critical_bit_index_ &&
             critical_bit_index_ < critical_bits_.size());
      GuessTask critical_bit = critical_bits_[critical_bit_index_++];
      if (!Guess(critical_bit)) BackTrack();
      if (critical_bit_index_ >= critical_bits_.size())
        guess_critical_bits_ = false;
      continue;
    }

    //! choose setting and guess a random bit
    if (GenerateSearchMasks(GetCurrentPhase().settings)) {
      GuessTask pos = ChooseGuessPos();
      if (!Guess(pos)) {
        AddCriticalBit({GetCurrentPhase().backtrack_guesses, pos.pos});
        BackTrack();
      }
      continue;
    }

    //! complete check
    if (GetCurrentPhase().twobit_complete) {
      int& complete_checks =
          current_status_.complete_data[current_status_.phase * 2];
      int& complete_success =
          current_status_.complete_data[current_status_.phase * 2 + 1];
      complete_checks++;
      if (complete_checks > complete_limit * (1 + complete_success)) {
        Restart();
        continue;
      }
      complete_check_ = true;
      if (!CompleteCheck(ComputeTwobitMask(2))) {
        current_status_.remaining_credits--;
        if (switched_back_or_stack_empty_) complete_check_ = true;
        continue;
      }
      complete_check_ = false;
      complete_success++;
    }

    //! crypto specific callback function
    if (!characteristic_.GetCrypto()->Callback(
            GetCurrentPhase().callback, characteristic_, rng_, logfile_)) {
      // Restart();
      BackTrack();
      // guess_critical_bits_ = false;
      continue;
    }

    //! if everything is ok, and it is the last phase, we found a characteristic
    if (config_.IsLastSearchPhase(current_status_.phase)) {
      current_status_.found++;
      // PrintInfo(false);
      characteristic_.GetCrypto()->Callback("found", characteristic_, rng_,
                                            logfile_);
      if (config_.GetSearchConfig().end_found) exit(0);
      Restart();
      continue;
    }

    //! otherwise we move to the next phase
    current_status_.phase++;
    search_stack_.push_back(characteristic_, current_status_.phase);
  }
  assert(!"something wrong here");
  exit(-1);
}

Search::GuessTask Search::ChooseGuessPos() {
  constexpr int initscore = 10000;
  const int branch = config_.GetSearchPhase(current_status_.phase).branch;

  // TODO include twobit_conditions
  std::string strategy =
      config_.GetSearchPhase(current_status_.phase).lookahead;
  if (branch && strategy == "none") strategy = "mcbranch";

  if (strategy == "none") {
    int setting = ChooseSetting();
    assert(setting >= 0);
    Bitpos startpos = GetSetting(setting).ordered_guesses
                          ? searchmasks_[setting].First()
                          : searchmasks_[setting].GetRandomBitpos(rng_);
    PosCandidate pos = {startpos, initscore, setting};
    return {GetSetting(pos.setting).guesses, pos.pos};
  } else if (strategy == "mcbranch" || strategy == "probbranch") {
    int setting = ChooseSetting();
    assert(setting >= 0);
    std::vector<PosCandidate> candidates;
    Bitpos startpos = GetSetting(setting).ordered_guesses
                          ? searchmasks_[setting].First()
                          : searchmasks_[setting].GetRandomBitpos(rng_);
    PosCandidate pos = {startpos, initscore, setting};
    int free_limit =
        searchmasks_[setting].GetNumBitsSet() / 2;  // TODO one limit per mask
    for (int i = 0; i < branch; ++i) {
      if (strategy == "mcbranch")
        EvaluatePos(pos, &Search::EvaluatePosValueBitsSet);
      else  // strategy == "probbranch"
        EvaluatePos(pos, &Search::EvaluatePosValueProb);

      candidates.emplace_back(pos);
      if (pos.value == -1)  // found a contradiction, return immediately
        return {GetSetting(pos.setting).guesses, pos.pos};
      // if ((pos.setting = ChooseSetting()) < 0) // searchmasks for all
      // settings empty
      if (searchmasks_[pos.setting].GetNumBitsSet() <= free_limit) break;
      pos.pos = searchmasks_[pos.setting].GetRandomBitpos(rng_);
    }
    std::sort(candidates.begin(), candidates.end(),
              [](const PosCandidate& first, const PosCandidate& second) {
                return first.value < second.value;
              });
    // std::uniform_int_distribution<int> dist(0, (candidates.size()/3));
    // const int index = dist(rng_);
    // pos = candidates[index];
    pos = candidates[0];
    return {GetSetting(pos.setting).guesses, pos.pos};
  } else {
    std::cerr << "Lookahead strategy \"" + strategy + "\" not implemented!"
              << std::endl;
    std::exit(-1);
  }
}

//! updates the characteristic with a random bit and calculates its value with
//! the callback function
void Search::EvaluatePos(PosCandidate& pos,
                         void (Search::*EvaluatePosValueFunc)(PosCandidate&)) {
  tmp_characteristic_.ShallowCopy(characteristic_);
  if (!SetBitRandom(pos))
    pos.value = -1;
  else {
    Bitmask& searchmask = searchmasks_[pos.setting];
    searchmask = characteristic_.GetConditionMask(
        searchmask,
        std::bind(IsGuessable, GetSetting(pos.setting), std::placeholders::_1));
    (this->*EvaluatePosValueFunc)(pos);
  }
  characteristic_.Undo();
  characteristic_.ShallowCopy(tmp_characteristic_);
}

//! evaluates the pos candidate based on the number of set bits
void Search::EvaluatePosValueBitsSet(PosCandidate& pos) {
  pos.value =
      characteristic_
          .GetConditionMask(initial_free_bits_mask_,
                            std::bind(IsGuessable, GetSetting(pos.setting),
                                      std::placeholders::_1))
          .GetNumBitsSet();
}

//! evaluates the pos candidate based on the probability
void Search::EvaluatePosValueProb(PosCandidate& pos) {
  pos.value =
      static_cast<int>(std::abs(characteristic_.GetProbabilitySum()) * 100);
}

bool Search::SetBitRandom(const PosCandidate& pos) {
  const std::vector<Config::Guess>& guesses = GetSetting(pos.setting).guesses;
  static std::uniform_real_distribution<double> real_rand(0.0, 1.0);
  const BitCondition bc = characteristic_.GetBitCondition(pos.pos);
  auto guess = std::find_if(guesses.begin(), guesses.end(),
                            [bc](Config::Guess g) { return g.bc == bc; });
  if (guess != guesses.end()) {
    int index = guess->choice_probability >= real_rand(real_rng_) ? 0 : 1;
    BitCondition cond = BitCondition(CHOICES[guess->bc].bc[index]);
    characteristic_.SetBitCondition(pos.pos, cond);
    return characteristic_.Update(true);
  }
  return true;
}

void Search::BackTrack() {
  assert(!search_stack_.empty());
  guess_critical_bits_ = critical_bits_.size() != 0;
  critical_bit_index_ = 0;
  int phase = BackTrackStrategy();

  switched_back_or_stack_empty_ = (phase != current_status_.phase);
  current_status_.phase = phase;
  switched_back_or_stack_empty_ |= search_stack_.empty();
  current_status_.remaining_credits--;
  current_status_.contradictions++;
}

int Search::BackTrackStrategy() {
  std::string strategy =
      config_.GetSearchPhase(current_status_.phase).backtrack;
  int phase = current_status_.phase;

  if (strategy == std::string("choice")) {
    std::tie(characteristic_, phase) =
        search_stack_.backtrack_to_other_choice();
  } else if (strategy == std::string("10perc")) {
    std::tie(characteristic_, phase) =
        search_stack_.backtrack(search_stack_.size() / 10);
  } else if (strategy == std::string("custom")) {
    // implement custom BackTrack Strategy
  }
  return phase;
}

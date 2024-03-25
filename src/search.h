#ifndef SEARCH_H_
#define SEARCH_H_

#include <cstdint>
#include <deque>
#include <map>
#include <random>
#include <vector>

#include "bitmask.h"
#include "bitpos.h"
#include "characteristic.h"
#include "search_stack.h"

class Logfile;
class XmlConfig;

/*!
 * \brief Main class for the search logic.
 */
class Search {
 public:
  /*!
   * \brief This object describes a possible candidate for a guess.
   *
   * It is used when evaluating the lookahead candidates.
   */
  struct PosCandidate {
    Bitpos pos;
    int value;
    int setting;
  };

  /*!
   * \brief This object describes the current status of the search.
   *
   * Some statistics about the search are collected and saved in this struct.
   * An overload for print output is provided.
   */
  struct Status {
    int64_t global_iterations;
    uint32_t seed;
    int start_time;
    int phase;
    int iterations;
    int stack_size;
    int contradictions;
    int restarts;
    int free_bits;
    int minfree;
    int absminfree;
    int remaining_credits;
    int found;
    int max_stack_size;
    std::vector<int> complete_data;
    int dump_time;
    int dump_count;

    void Init() {
      iterations = 0;
      stack_size = 0;
      contradictions = 0;
      free_bits = 0x7fffffff;
      minfree = 0x7fffffff;
      max_stack_size = 0;
      complete_data.clear();
      complete_data.resize(100, 0);
    }

    friend std::ostream& operator<<(std::ostream& os, const Status& ss) {
      int run_time = time(0) - ss.start_time;
      double iterations_per_second =
          (run_time != 0) ? (double(ss.global_iterations) / double(run_time))
                          : 0;
      int precision = iterations_per_second < 10
                          ? 2
                          : iterations_per_second < 1
                                ? 3
                                : iterations_per_second < 0.1 ? 4 : -1;
      os << "Info:";
      os << " seed: " << ss.seed;
      os << " time: " << run_time;
      // os << " global_iterations: " << ss.global_iterations;
      if (iterations_per_second < 10)
        os << " iterations/sec: " << std::setprecision(precision)
           << iterations_per_second << std::setprecision(-1);
      else
        os << " iterations/sec: " << int(iterations_per_second);
      os << " iterations: " << ss.iterations;
      os << " stack_size: " << ss.stack_size;
      os << " contr: " << ss.contradictions;
      os << " restarts: " << ss.restarts;
      // os << " free_bits: " << ss.free_bits;
      os << " minfree: " << ss.minfree;
      os << " absminfree: " << ss.absminfree;
      os << " credits: " << ss.remaining_credits;
      os << " phase: " << ss.phase;
      os << " found: " << ss.found;
      os << " smax: " << ss.max_stack_size;
      os << " complete:";
      for (int i = 0; i <= ss.phase; i++) {
        os << " " << ss.complete_data[i * 2 + 1];
        os << "/" << ss.complete_data[i * 2];
      }
      return os;
    }
  };

  /*!
   * \brief Representation of the two choices when guessing a BitCondition.
   *
   * See \see CHOICES for the predefined choices.
   */
  struct Choice {
    char bc[2][2];
    double choice_probability;
  };

  //! Predefined Choices
  static constexpr Choice CHOICES[16] = {
      //      1st  2nd   choice  stack
      {{"#", "#"}, 0},    // '#'
      {{"0", "0"}, 0},    // '0'
      {{"u", "u"}, 0},    // 'u'
      {{"0", "u"}, 1},    // '3'
      {{"n", "n"}, 0},    // 'n'
      {{"0", "n"}, 1},    // '5'
      {{"u", "n"}, 0.5},  // 'x'
      {{"0", "x"}, 1},    // '7'
      {{"1", "1"}, 0},    // '1'
      {{"0", "1"}, 0.5},  // '-'
      {{"1", "u"}, 1},    // 'A'
      {{"-", "u"}, 1},    // 'B'
      {{"1", "n"}, 1},    // 'C'
      {{"-", "n"}, 1},    // 'D'
      {{"1", "x"}, 1},    // 'E'
      {{"-", "x"}, 1}     // '?'
  };

  /*!
   * \brief Representation of the search config from the XML configuration
   * file.
   */
  struct Config {
    struct Mask {
      Bitmask bitmask;
      int twobit_threshold;
    };
    struct Guess {
      BitCondition bc;
      double choice_probability;
    };
    struct Setting {
      double probability;
      std::vector<Mask> masks;
      std::vector<Guess> guesses;
      bool ordered_guesses;
    };
    struct Phase {
      bool twobit_complete;
      int branch;
      std::string callback;
      std::string backtrack;
      std::string lookahead;
      std::vector<Setting> settings;
      std::vector<Guess> backtrack_guesses;
    };

    int seed;
    int reseed;
    int credits;
    std::string callback;
    bool end_found;
    int end_time;
    int print_whole_characteristic;
    int print_info;
    int print_characteristic;
    int print_minfree;
    std::vector<Phase> phases;
  };

  /*!
   * \brief Representation of a guess of a bit, used to save critical bits.
   */
  struct GuessTask {
    std::vector<Config::Guess> guesses;
    Bitpos pos;
  };

  Search(XmlConfig& config, Characteristic& s, Logfile& logfile,
         int stack_fullstate_frequency, int64_t seed = -1);
  ~Search();

  void PrintInfo(bool print = false);

  void Restart();
  bool CompleteCheck(Bitmask mask);
  Bitmask ComputeTwobitMask(int threshold);

  void UpdateStatus();
  bool UpdateMinfree();
  void BackTrack();
  void AddCriticalBit(GuessTask bit);
  bool Guess(GuessTask pos);
  bool GuessBit(Config::Guess guess, Bitpos pos);
  void Start();

  GuessTask ChooseGuessPos();
  bool SetBitRandom(const PosCandidate& pos);
  void EvaluatePos(PosCandidate& pos,
                   void (Search::*EvaluatePosValueFunc)(PosCandidate&));
  void EvaluatePosValueBitsSet(PosCandidate& pos);
  void EvaluatePosValueProb(PosCandidate& pos);

  void SetDumpTime(int dump_time);
  void SetDumpCount(int dump_count);
  void CheckforDump();

 private:
  const Config::Phase& GetCurrentPhase();
  const Config::Phase& GetLastPhase();
  const Config::Setting& GetSetting(int index);
  int ChooseSetting();
  int BackTrackStrategy();
  static bool IsGuessable(const Config::Setting& setting, BitCondition bc);
  bool GenerateSearchMasks(const std::vector<Config::Setting>& settings);
  Bitmask GenerateWordMask(const Config::Setting& setting);

  std::deque<GuessTask> critical_bits_;
  bool twobit_conditions_generated_;
  bool complete_check_;
  XmlConfig& config_;

  Characteristic start_characteristic_;
  Characteristic tmp_characteristic_;
  Characteristic& characteristic_;
  std::vector<Bitmask> searchmasks_;
  Bitmask initial_free_bits_mask_;
  SearchStack search_stack_;

  Status current_status_;
  Logfile& logfile_;
  bool guess_critical_bits_;
  bool switched_back_or_stack_empty_;
  int critical_bit_index_;
  int credits_;
  uint32_t seed_;
  int seed_time_;
  int print_time_;
  int print_characteristic_time_;

  std::mt19937 rng_;
  std::ranlux24 real_rng_;
};

#endif  // SEARCH_H_

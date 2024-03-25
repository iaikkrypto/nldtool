#ifndef NLDTOOL_SEARCH_STACK_H
#define NLDTOOL_SEARCH_STACK_H

#include <map>
#include <utility>
#include <vector>

#include "bitpos.h"
#include "characteristic.h"
#include "condition.h"

/*!
 * \brief A custom stack for the current state of the \see Characteristic
 * during the search.
 *
 * The stack functions in the following way: It only stores the changes (bit
 * guesses) for each step of the search, not the whole \see Characteristic,
 * since that can lead to large memory consumption. Only every
 * full_save_frequency guesses, the whole \see Characteristic is placed on the
 * stack. The wanted state for pop operations is build back up from the last
 * full state and the guesses following it.
 */
class SearchStack {
  typedef std::pair<Bitpos, std::pair<BitCondition, BitCondition>>
      guess_container;

  /*!
   * \brief  The type of the stack's element.
   *
   * A StackElement consists of a full \see Characteristic and all guesses
   * following it. Only full_save_frequency guesses are allowed per
   * StackElement, then a new one is created.
   */
  struct StackElement {
    Characteristic characteristic;
    std::vector<guess_container> guesses;
    int phase;

    StackElement(const Characteristic& c, int phase)
        : characteristic(c), guesses(), phase(phase) {}
    StackElement(StackElement&& s)
        : characteristic(std::move(s.characteristic)),
          guesses(std::move(s.guesses)),
          phase(s.phase) {}
    StackElement() = delete;
    StackElement(const StackElement&) = delete;
    StackElement& operator=(const StackElement&) = delete;
    StackElement& operator=(StackElement&&) = delete;
  };

 public:
  SearchStack(int full_save_frequency = 20) : frequency_(full_save_frequency) {}

  void push_back(const Characteristic& characteristic, int phase);
  void add_guess(const Characteristic& c, const Bitpos& pos,
                 const BitCondition& cond);
  void add_guess(const Characteristic& c, const Bitpos& pos,
                 const BitCondition& cond1, const BitCondition& cond2);
  std::pair<Characteristic, int> get_pop_back();
  std::pair<Characteristic, int> backtrack(int steps);
  std::pair<Characteristic, int> backtrack_to_other_choice();
  void clear();
  int size();
  bool empty();

 private:
  guess_container back_guess();
  std::vector<StackElement> search_stack_;
  int frequency_;
};

#endif  // NLDTOOL_SEARCH_STACK_H

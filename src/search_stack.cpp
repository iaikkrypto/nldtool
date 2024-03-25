#include "search_stack.h"

#include <cassert>

void SearchStack::clear() { search_stack_.clear(); }

bool SearchStack::empty() { return size() == 0; }

int SearchStack::size() {
  int size = 0;
  for (auto& el : search_stack_) size += el.guesses.size();
  return size;
}

SearchStack::guess_container SearchStack::back_guess() {
  assert(!empty());  // given by caller
  while (search_stack_.back().guesses.empty()) {
    search_stack_.pop_back();
  }

  auto ret = search_stack_.back().guesses.back();
  search_stack_.back().guesses.pop_back();
  return ret;
}

void SearchStack::push_back(const Characteristic& characteristic, int phase) {
  search_stack_.push_back(StackElement(characteristic, phase));
}

void SearchStack::add_guess(const Characteristic& c, const Bitpos& pos,
                            const BitCondition& cond) {
  assert(!search_stack_.empty());
  search_stack_.back().guesses.emplace_back(
      pos, std::make_pair(cond, BitCondition("#")));

  // every frequency_ guesses, put a copy of the characteristic on the stack
  if (search_stack_.back().guesses.size() >= frequency_) {
    push_back(c, search_stack_.back().phase);
  }
}

void SearchStack::add_guess(const Characteristic& c, const Bitpos& pos,
                            const BitCondition& cond1,
                            const BitCondition& cond2) {
  assert(!search_stack_.empty());
  search_stack_.back().guesses.emplace_back(pos, std::make_pair(cond1, cond2));

  // every frequency_ guesses, put a copy of the characteristic on the stack
  if (search_stack_.back().guesses.size() >= frequency_) {
    push_back(c, search_stack_.back().phase);
  }
}

std::pair<Characteristic, int> SearchStack::get_pop_back() {
  assert(!search_stack_.empty());
  std::pair<Characteristic, int> ret = std::make_pair(
      search_stack_.back().characteristic, search_stack_.back().phase);
  for (auto& guess : search_stack_.back().guesses) {
    ret.first.SetBitCondition(guess.first, guess.second.first);
  }
  assert(ret.first.Update(true));  // there should only be doable guesses on the
                                   // first pos of the stack

  if (search_stack_.back().guesses.empty()) {
    search_stack_.pop_back();
    if (!search_stack_.empty() && !search_stack_.back().guesses.empty())
      search_stack_.back().guesses.pop_back();
  } else
    search_stack_.back().guesses.pop_back();

  return ret;
}

std::pair<Characteristic, int> SearchStack::backtrack(int steps) {
  assert(!search_stack_.empty());

  while (steps > search_stack_.back().guesses.size() &&
         search_stack_.size() > 1) {
    steps -= search_stack_.back().guesses.size();
    search_stack_.pop_back();
  }
  std::pair<Characteristic, int> ret = std::make_pair(
      search_stack_.back().characteristic, search_stack_.back().phase);
  if (steps > search_stack_.back().guesses.size()) {
    search_stack_.back().guesses.clear();
    return ret;
  }

  // discard "steps" guesses
  search_stack_.back().guesses.resize(search_stack_.back().guesses.size() -
                                      steps);

  // redo guesses
  for (auto& guess : search_stack_.back().guesses) {
    ret.first.SetBitCondition(guess.first, guess.second.first);
  }
  assert(ret.first.Update(
      true));  // there should only be doable guesses on the stack
  return ret;
}

std::pair<Characteristic, int> SearchStack::backtrack_to_other_choice() {
  assert(!search_stack_.empty());

  auto selector = [](std::pair<BitCondition, BitCondition> conds) {
    return conds.second == BitCondition("#");
  };
  do {
    if (empty()) {
      return std::make_pair(search_stack_.back().characteristic,
                            search_stack_.back().phase);
    }
    // pop all that do not have a second choice
    guess_container other_choice;
    for (other_choice = back_guess(); selector(other_choice.second);
         other_choice = back_guess()) {
      if (empty()) {
        return std::make_pair(search_stack_.back().characteristic,
                              search_stack_.back().phase);
      }
    }

    std::pair<Characteristic, int> ret = std::make_pair(
        search_stack_.back().characteristic, search_stack_.back().phase);

    // redo guesses
    for (auto& guess : search_stack_.back().guesses) {
      ret.first.SetBitCondition(guess.first, guess.second.first);
    }

    // use other choice
    ret.first.SetBitCondition(other_choice.first, other_choice.second.second);

    if (ret.first.Update(true)) {
      add_guess(ret.first, other_choice.first, other_choice.second.second);
      return ret;
    }
  } while (1);
}

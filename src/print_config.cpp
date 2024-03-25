#include "print_config.h"

PrintConfig::PrintConfig() {}

void PrintConfig::SetFlag(std::string name, bool value) {
  flags_[name] = value;
}

bool PrintConfig::ContainsFlag(std::string name) const {
  return flags_.find(name) != flags_.end();
}

bool PrintConfig::GetFlag(std::string name) const {
  if (ContainsFlag(name))
    return flags_.find(name)->second;
  else
    return false;
}

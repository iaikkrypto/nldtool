#ifndef PRINT_CONFIG_H_
#define PRINT_CONFIG_H_

#include <map>
#include <string>

/*!
 * \brief Class containing all print flags and their status.
 */
class PrintConfig {
  std::map<std::string, bool> flags_;

 public:
  PrintConfig();

  void SetFlag(std::string name, bool value);

  bool ContainsFlag(std::string name) const;

  bool GetFlag(std::string name) const;
};

#endif  // PRINT_CONFIG_H_

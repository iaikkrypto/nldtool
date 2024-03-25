#ifndef XML_CONFIG_H_
#define XML_CONFIG_H_

#include <cstdint>
#include <cstring>
#include <map>
#include <memory>
#include <sstream>
#include <vector>

#include "print_config.h"
#include "search.h"

namespace tinyxml2 {
class XMLElement;
}

namespace cxxopts {
class Options;
}

class Characteristic;
class Crypto;
class Logfile;

/*!
 * \brief Class responsible for parsing XML config files and storing the set
 * options.
 *
 * Tasks include the parsing of the search configuration and print
 * configuration and providing a global access point to all options (set via XML
 * or command line).
 */
class XmlConfig {
 public:
  XmlConfig(cxxopts::Options& options, bool override_old_options = false);

  ~XmlConfig();

  bool SetWordMasksRegex(Search::Config::Mask& mask, const std::string& words,
                         const std::string& rounds);

  void AddBackTrack(Search::Config::Phase& phase);

  std::stringstream& GetCharacteristicStream();

  cxxopts::Options& GetOptions();

  PrintConfig& GetPrintConfig();

  Search::Config& GetSearchConfig();

  bool GetReadMainSteps() const;

  bool GetReadAllSteps() const;

  bool GetPrintMainSteps() const;

  bool GetPrintAllSteps() const;

  const Search::Config::Phase& GetSearchPhase(int level) const;

  bool IsLastSearchPhase(int level) const;

  void LoadFromString(const std::string& str);

  void Load(const std::string& configfile);

  bool CheckIfSearchIsDefined(const std::string& configfile);

  void GenerateCrypto();

  Characteristic GenerateCharacteristic();

  void PrintSearchConfig(Logfile& logfile);

  void PrintOptions(Logfile& logfile);

 private:
  void InsertExternalPrintConfig(const std::string& print_config_file,
                                 bool warning_if_not_found,
                                 tinyxml2::XMLElement* root);

  void InsertExternalSearchConfig(const std::string& searchconfigfile,
                                  bool warning_if_not_found,
                                  tinyxml2::XMLElement* root);

  static void Error(const char* format, ...);

  static void Warning(const char* format, ...);

  void HandleOptions(tinyxml2::XMLElement* option);

  void HandleCharacteristic(tinyxml2::XMLElement* characteristic);

  void HandleSearch(tinyxml2::XMLElement* xml_search);

  void HandlePhases(tinyxml2::XMLElement* xml_phase);

  void HandleSetting(Search::Config::Setting& setting,
                     tinyxml2::XMLElement* xml_setting);

  void HandleMasks(Search::Config::Setting& setting,
                   tinyxml2::XMLElement* xml_mask);

  void HandleGuesses(Search::Config::Setting& setting,
                     tinyxml2::XMLElement* xml_guess);

  void HandlePrintConfig(tinyxml2::XMLElement* config);

  bool override_;
  cxxopts::Options& options_;
  std::stringstream characteristicstream_;
  PrintConfig print_config_;
  Search::Config searchconfig_;
  std::shared_ptr<Crypto> crypto_;
  Characteristic* characteristic_;
};
#endif  // XML_CONFIG_H_

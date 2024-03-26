#include "xml_config.h"

#include <cstdarg>

#include "crypto.h"
#include "crypto_factory.h"
#include "logfile.h"
#include "text_io_format.h"
#include "tinyxml2.h"

// TODO: maybe start pull request into tinyxml2
tinyxml2::XMLNode* DeepCopy(const tinyxml2::XMLNode* org,
                            tinyxml2::XMLDocument* newDoc) {
  tinyxml2::XMLNode* parentCopy = org->ShallowClone(newDoc);
  for (const tinyxml2::XMLNode* child = org->FirstChild(); child;
       child = child->NextSibling()) {
    tinyxml2::XMLNode* childCopy = DeepCopy(child, newDoc);
    parentCopy->InsertEndChild(childCopy);
  }
  return parentCopy;
}

bool XmlConfig::SetWordMasksRegex(Search::Config::Mask& mask,
                                  const std::string& words,
                                  const std::string& rounds) {
  try {
    std::regex word_regex(words);
    if (rounds.empty()) {
      mask.bitmask = crypto_->GetConditionWordMaskRegex(word_regex);
      return true;
    }

    int start, end, index;
    index = rounds.find('-');
    if (index == std::string::npos) {
      // single step number
      start = end = std::stoi(rounds);
    } else {
      // range of step numbers
      index++;
      start = std::stoi(rounds.substr(0, index));
      end = std::stoi(rounds.substr(index, rounds.length() - index));
    }
    mask.bitmask =
        crypto_->GetConditionWordIntervalMaskRegex(word_regex, start, end);
    return true;
  } catch (std::regex_error& e) {
    std::cerr << "invalid regex for word: " << e.what() << std::endl;
    return false;
  }
}

void XmlConfig::AddBackTrack(Search::Config::Phase& phase) {
  // add backtracking guesses from previous phase
  if (!searchconfig_.phases.empty()) {
    Search::Config::Phase previous = searchconfig_.phases.back();
    phase.backtrack_guesses.insert(phase.backtrack_guesses.end(),
                                   previous.backtrack_guesses.begin(),
                                   previous.backtrack_guesses.end());
  }
  for (int i = 0; i < phase.settings.size(); ++i) {
    // add guesses from settings to backtracking guesses
    Search::Config::Setting setting = phase.settings[i];
    for (auto it = setting.guesses.begin(); it != setting.guesses.end(); ++it) {
      if (std::find_if(
              phase.backtrack_guesses.begin(), phase.backtrack_guesses.end(),
              [it](Search::Config::Guess g) { return g.bc == it->bc; }) ==
          phase.backtrack_guesses.end()) {
        Search::Config::Guess guess;
        guess.bc = it->bc;
        guess.choice_probability = Search::CHOICES[guess.bc].choice_probability;
        phase.backtrack_guesses.push_back(guess);
        // if we guess on '?', we also add '3', '5', '7', 'A', 'B', 'C', 'D',
        // 'E'
        if (guess.bc == BitCondition("?")) {
          const int index[8] = {0x3, 0x5, 0x7, 0xA, 0xB, 0xC, 0xD, 0xE};
          for (int i = 0; i < 8; i++) {
            Search::Config::Guess guess;
            guess.bc = BitCondition(index[i]);
            guess.choice_probability =
                Search::CHOICES[guess.bc].choice_probability;
            phase.backtrack_guesses.push_back(guess);
          }
        }
      }
    }
  }
}

XmlConfig::XmlConfig(cxxopts::Options& options, bool override_old_options)
    : override_(override_old_options),
      options_(options),
      characteristicstream_(std::stringstream::in | std::stringstream::out),
      crypto_(nullptr),
      characteristic_(nullptr) {}

XmlConfig::~XmlConfig() { delete characteristic_; }

std::stringstream& XmlConfig::GetCharacteristicStream() {
  return characteristicstream_;
}

cxxopts::Options& XmlConfig::GetOptions() { return options_; }

PrintConfig& XmlConfig::GetPrintConfig() { return print_config_; }

Search::Config& XmlConfig::GetSearchConfig() { return searchconfig_; }

bool XmlConfig::GetReadMainSteps() const {
  return (options_["read-steps"].as<std::string>().compare("main") == 0);
}

bool XmlConfig::GetReadAllSteps() const {
  return (options_["read-steps"].as<std::string>().compare("all") == 0);
}

bool XmlConfig::GetPrintMainSteps() const {
  return (options_["print-steps"].as<std::string>().compare("main") == 0);
}

bool XmlConfig::GetPrintAllSteps() const {
  return (options_["print-steps"].as<std::string>().compare("all") == 0);
}

const Search::Config::Phase& XmlConfig::GetSearchPhase(int level) const {
  assert(level >= 0 && level < searchconfig_.phases.size());
  return searchconfig_.phases[level];
}

bool XmlConfig::IsLastSearchPhase(int level) const {
  return level >= searchconfig_.phases.size() - 1;
}

bool XmlConfig::CheckIfSearchIsDefined(const std::string& configfile) {
  std::shared_ptr<tinyxml2::XMLDocument> doc(new tinyxml2::XMLDocument());
  tinyxml2::XMLError err = doc->LoadFile(configfile.c_str());
  if (err != tinyxml2::XMLError::XML_SUCCESS) {
    return false;
  }
  tinyxml2::XMLElement* root =
      doc->FirstChildElement("config")
          ? doc->FirstChildElement("config")->ToElement()
          : NULL;
  if (root == NULL) return false;
  tinyxml2::XMLElement* node = root->FirstChildElement("search");
  if (node == NULL) {
    std::shared_ptr<tinyxml2::XMLDocument> doc_search_config_file(
        new tinyxml2::XMLDocument());
    if (doc_search_config_file->LoadFile(
            options_["search-config"].as<std::string>().c_str()) !=
        tinyxml2::XMLError::XML_SUCCESS)
      return false;
  }
  return true;
}

void XmlConfig::Load(const std::string& configfile) {
  std::shared_ptr<tinyxml2::XMLDocument> doc(new tinyxml2::XMLDocument());
  tinyxml2::XMLError err = doc->LoadFile(configfile.c_str());
  if (err != tinyxml2::XMLError::XML_SUCCESS) {
    // doc->PrintError();
    if (err == tinyxml2::XMLError::XML_ERROR_FILE_COULD_NOT_BE_OPENED ||
        err == tinyxml2::XMLError::XML_ERROR_FILE_NOT_FOUND ||
        err == tinyxml2::XMLError::XML_ERROR_FILE_READ_ERROR)
      Error("could not open ('%s')", configfile.c_str());
    else
      Error("%s: %s %s", configfile.c_str(), doc->GetErrorStr1(),
            doc->GetErrorStr2());
  }
  tinyxml2::XMLElement* root =
      doc->FirstChildElement("config")
          ? doc->FirstChildElement("config")->ToElement()
          : NULL;
  if (root == NULL) Error("has to start with <config>");
  tinyxml2::XMLElement* node = root->FirstChildElement("options");
  if (node == NULL)
    Warning("no <options> defined!");
  else
    HandleOptions(node);
  InsertExternalPrintConfig(options_["print-config"].as<std::string>(),
                            options_.count("print-config"), root);
  InsertExternalSearchConfig(options_["search-config"].as<std::string>(),
                             options_.count("search-config"), root);
  tinyxml2::XMLPrinter p;
  root->Accept(&p);
  std::cout << p.CStr();
  node = root->FirstChildElement("char");
  if (node == NULL)
    Error("char declaration missing (define with <char value=\"...\">)");
  HandleCharacteristic(node);
  node = root->FirstChildElement("print_config");
  if (node == NULL)
    Warning("no <print_config> defined!");
  else
    HandlePrintConfig(node);
  node = root->FirstChildElement("search");
  if (node == NULL) {
    if (options_.count("check-char") == 0) {
      Warning("no <search> defined! only checking characteristic");
      const_cast<cxxopts::OptionDetails&>(options_["check-char"]).parse("1");
    }
    return;
  }
  HandleSearch(node);
}

void XmlConfig::LoadFromString(const std::string& str) {
  std::cout << "\n\n\n\n\n\n\n\n LoadFromString: " << str << std::endl;
  std::shared_ptr<tinyxml2::XMLDocument> doc(new tinyxml2::XMLDocument());
  doc->Parse(str.c_str());
  // Declare a printer
  tinyxml2::XMLPrinter printer;
  // attach it to the document you want to convert in to a std::string
  doc->Accept(&printer);
  // Create a std::string and copy your document data in to the string
  // std::string out = printer.c_str();
  // std::cout <<"\n\n\n\n\n\n\n\n\nOUT: "<<out<<std::endl;
  tinyxml2::XMLElement* root =
      doc->FirstChildElement("config")
          ? doc->FirstChildElement("config")->ToElement()
          : NULL;
  if (root == NULL) Error("has to start with <config>");
  tinyxml2::XMLElement* node = root->FirstChildElement("options");
  if (node == NULL) Warning("no <options> defined!");
  HandleOptions(node);
  InsertExternalPrintConfig(options_["print-config"].as<std::string>(),
                            options_.count("print-config"), root);
  InsertExternalSearchConfig(options_["search-config"].as<std::string>(),
                             options_.count("search-config"), root);
  node = root->FirstChildElement("char");
  if (node == NULL)
    Error("char declaration missing (define with <char value=\"...\">)");
  HandleCharacteristic(node);
  node = root->FirstChildElement("print_config");
  if (node == NULL)
    Warning("no <print_config> defined!");
  else
    HandlePrintConfig(node);
  node = root->FirstChildElement("search");
  if (node == NULL) {
    if (options_.count("check-char") == 0) {
      Warning("no <search> defined! only checking characteristic");
      const_cast<cxxopts::OptionDetails&>(options_["check-char"]).parse("1");
    }
    return;
  }
  HandleSearch(node);
}

void XmlConfig::PrintSearchConfig(Logfile& logfile) {
  // print search config
  std::ifstream in;
  bool b = false;
  in.open(options_["search-config"].as<std::string>().c_str());
  if (in.is_open()) {
    while (!in.eof()) {
      char c[1024];
      in.getline(c, 1024);
      std::string l(c);
      if (l.find("<search") != std::string::npos) b = true;
      if (b) {
        logfile << l << "\n";
      }
      if (l.find("</search>") != std::string::npos) break;
    }
  }
  in.close();
  if (!b) {
    in.open(options_["input-file"].as<std::string>().c_str());
    while (!in.eof()) {
      char c[256];
      in.getline(c, 1024);
      std::string l(c);
      if (l.find("<search") != std::string::npos) b = true;
      if (b) {
        logfile << l << "\n";
      }
      if (l.find("</search>") != std::string::npos) break;
    }
    in.close();
  }
}

void XmlConfig::PrintOptions(Logfile& logfile) {
  /*TODO: add functionality to print all params
  for (auto stringparam : options_.GetStringOptions())
    logfile << stringparam.first << " " << stringparam.second << "  ";
  for (auto intparam : options_.GetIntegerOptions())
    logfile << intparam.first << " " << (int)intparam.second << "  ";
  std::cout << std::endl;
  */
}

void XmlConfig::GenerateCrypto() {
  crypto_ = std::shared_ptr<Crypto>(CryptoFactory(options_));
}

Characteristic XmlConfig::GenerateCharacteristic() {
  if (!crypto_) GenerateCrypto();
  crypto_->GetTextIOFormat()->SetPrintConfig(GetPrintConfig());
  if (GetPrintMainSteps()) crypto_->GetTextIOFormat()->SetPrintOnlyMainSteps();
  if (GetPrintAllSteps()) crypto_->GetTextIOFormat()->SetPrintMainAndSubSteps();
  if (!characteristic_ && !options_["input-file"].as<std::string>().empty()) {
    characteristic_ = new Characteristic(crypto_);
    characteristic_->ReadCharacteristic(GetCharacteristicStream(),
                                        GetReadMainSteps());
  }
  return *characteristic_;
}

void XmlConfig::InsertExternalPrintConfig(const std::string& print_config_file,
                                          bool warning_if_not_found,
                                          tinyxml2::XMLElement* root) {
  assert(root);

  std::shared_ptr<tinyxml2::XMLDocument> doc(new tinyxml2::XMLDocument());

  tinyxml2::XMLError err = doc->LoadFile(print_config_file.c_str());
  if (err != tinyxml2::XMLError::XML_SUCCESS) {
    if (err == tinyxml2::XMLError::XML_ERROR_FILE_COULD_NOT_BE_OPENED ||
        err == tinyxml2::XMLError::XML_ERROR_FILE_NOT_FOUND ||
        err == tinyxml2::XMLError::XML_ERROR_FILE_READ_ERROR) {
      if (warning_if_not_found)
        Warning("could not open ('%s')", print_config_file.c_str());
      return;
    } else
      Error("%s: %s %s", print_config_file.c_str(), doc->GetErrorStr1(),
            doc->GetErrorStr2());
  } else
    printf("loading local print config file ('%s')\n",
           print_config_file.c_str());
  tinyxml2::XMLElement* pcl =
      doc->FirstChildElement("print_config")
          ? doc->FirstChildElement("print_config")->ToElement()
          : NULL;
  if (pcl == NULL) Error("has to start with <print_config>");
  tinyxml2::XMLElement* pcg =
      root->FirstChildElement("print_config")
          ? root->FirstChildElement("print_config")->ToElement()
          : NULL;
  // no print config in config file specified
  if (pcg == NULL) {
    root->InsertEndChild(DeepCopy(pcl, root->GetDocument()));
    return;
  }
  // empty print config in config file specified -> add
  if (pcg->FirstChild() == NULL) {
    pcl = pcl->FirstChildElement();
    while (pcl) {
      pcg->InsertEndChild(DeepCopy(pcl, pcg->GetDocument()));
      pcl = pcl->NextSiblingElement();
    }
    return;
  }
  // a print config in config file is specified -> add as last to overwrite
  // config file
  pcl = pcl->FirstChildElement();
  while (pcl) {
    pcg->InsertAfterChild(pcg->LastChild(), DeepCopy(pcl, root->GetDocument()));
    pcl = pcl->NextSiblingElement();
  }
}

void XmlConfig::InsertExternalSearchConfig(const std::string& searchconfigfile,
                                           bool warning_if_not_found,
                                           tinyxml2::XMLElement* root) {
  assert(root);
  std::shared_ptr<tinyxml2::XMLDocument> doc(new tinyxml2::XMLDocument());
  tinyxml2::XMLError err = doc->LoadFile(searchconfigfile.c_str());
  if (err != tinyxml2::XMLError::XML_SUCCESS) {
    if (err == tinyxml2::XMLError::XML_ERROR_FILE_COULD_NOT_BE_OPENED ||
        err == tinyxml2::XMLError::XML_ERROR_FILE_NOT_FOUND ||
        err == tinyxml2::XMLError::XML_ERROR_FILE_READ_ERROR) {
      if (warning_if_not_found)
        Warning("could not open ('%s')", searchconfigfile.c_str());
      return;
    } else
      Error("%s: %s %s", searchconfigfile.c_str(), doc->GetErrorStr1(),
            doc->GetErrorStr2());
  } else
    printf("loading local search config file ('%s')\n", doc->Value());
  tinyxml2::XMLElement* scl =
      doc->FirstChildElement("search")
          ? doc->FirstChildElement("search")->ToElement()
          : NULL;
  if (scl == NULL) Error("has to start with <search>");
  tinyxml2::XMLElement* scg =
      root->FirstChildElement("search")
          ? root->FirstChildElement("search")->ToElement()
          : NULL;
  // remove old search config
  if (scg != NULL) root->DeleteChild(scg);
  // add new search config
  root->InsertEndChild(DeepCopy(scl, root->GetDocument()));
}

void XmlConfig::Error(const char* format, ...) {
  va_list argptr;
  va_start(argptr, format);
  printf("ERROR: XML config - ");
  vprintf(format, argptr);
  printf("\n");
  va_end(argptr);
  exit(-1);
}

void XmlConfig::Warning(const char* format, ...) {
  va_list argptr;
  va_start(argptr, format);
  printf("WARNING: XML config - ");
  vprintf(format, argptr);
  printf("\n");
  va_end(argptr);
}

void XmlConfig::HandleOptions(tinyxml2::XMLElement* option) {
  tinyxml2::XMLElement* e = option->FirstChildElement();
  if (e == NULL) return;
  do {
    std::string name = e->Attribute("name") ? e->Attribute("name") : "";
    std::string value = e->Attribute("value") ? e->Attribute("value") : "";
    if (value.length() == 0 || name.length() == 0)
      Error(
          "invalid option definition; all options need a name and value "
          "attribute");
    if (override_ == true || options_.count(name) == 0) {
      const_cast<cxxopts::OptionDetails&>(options_[name]).parse(value);
    }
    e = e->NextSiblingElement();
  } while (e);
  GenerateCrypto();
}

void XmlConfig::HandleCharacteristic(tinyxml2::XMLElement* characteristic) {
  if (characteristic->Attribute("value") == NULL)
    Error("an attribute 'value' must be defined in element <char>");
  characteristicstream_ << characteristic->Attribute("value");
}

void XmlConfig::HandleSearch(tinyxml2::XMLElement* xml_search) {
  // set search options
  searchconfig_.end_found = options_["end-found"].as<bool>();
  searchconfig_.end_time = options_["end-time"].as<int>();
  searchconfig_.print_whole_characteristic = 0;
  searchconfig_.print_info = options_["print-info"].as<int>();
  searchconfig_.print_characteristic = options_["print-char"].as<int>();
  searchconfig_.print_minfree = options_["minfree-threshold"].as<int>();
  // default search attributes
  searchconfig_.seed = -1;
  searchconfig_.reseed = -1;
  searchconfig_.credits = -1;
  searchconfig_.callback = "";
  // parse search attributes
  for (auto a = xml_search->FirstAttribute(); a != 0; a = a->Next()) {
    if (strcmp(a->Name(), "seed") == 0) {
      searchconfig_.seed = std::atoi(a->Value());
    } else if (strcmp(a->Name(), "reseed") == 0) {
      searchconfig_.reseed = std::atoi(a->Value());
    } else if (strcmp(a->Name(), "credits") == 0) {
      searchconfig_.credits = std::atoi(a->Value());
    } else if (strcmp(a->Name(), "callback") == 0) {
      searchconfig_.callback = a->Value();
    } else {
      Error("unknown attribute '%s' in element <search>", a->Name());
    }
  }
  if (searchconfig_.credits < 0)
    Error("an attribute 'credits' must be defined in element <search>");
  tinyxml2::XMLElement* p = xml_search->FirstChildElement();
  if (p == NULL)
    Error("a child element <phase> must be defined in element <search>");
  HandlePhases(p);
}

void XmlConfig::HandlePhases(tinyxml2::XMLElement* xml_phase) {
  // parse phases
  do {
    // default phase attributes
    Search::Config::Phase phase;
    phase.backtrack = "choice";
    phase.lookahead = "none";
    phase.callback = "";
    // parse phase attributes
    for (auto a = xml_phase->FirstAttribute(); a != 0; a = a->Next()) {
      if (strcmp(a->Name(), "twobit_complete") == 0) {
        phase.twobit_complete = std::atoi(a->Value());
      } else if (strcmp(a->Name(), "branch") == 0) {
        phase.branch = std::atoi(a->Value());
      } else if (strcmp(a->Name(), "mcbranch") == 0) {
        phase.lookahead = "mcbranch";
        phase.branch = std::atoi(a->Value());
      } else if (strcmp(a->Name(), "backtrack") == 0) {
        phase.backtrack = a->Value();
      } else if (strcmp(a->Name(), "lookahead") == 0) {
        phase.lookahead = a->Value();
      } else if (strcmp(a->Name(), "callback") == 0) {
        phase.callback = a->Value();
      } else {
        Error("unknown attribute '%s' in element <phase>", a->Name());
      }
    }
    // parse settings
    tinyxml2::XMLElement* xml_setting = xml_phase->FirstChildElement("setting");
    while (xml_setting) {
      Search::Config::Setting setting;
      HandleSetting(setting, xml_setting);
      phase.settings.push_back(setting);
      xml_setting = xml_setting->NextSiblingElement("setting");
    }
    AddBackTrack(phase);
    searchconfig_.phases.push_back(phase);
    xml_phase = xml_phase->NextSiblingElement();
  } while (xml_phase);
}

void XmlConfig::HandleSetting(Search::Config::Setting& setting,
                              tinyxml2::XMLElement* xml_setting) {
  // try to find root nodes of words and guesses to get first child node
  tinyxml2::XMLElement* xml_mask = xml_setting->FirstChildElement("mask");
  tinyxml2::XMLElement* xml_guess = xml_setting->FirstChildElement("guess");
  if (xml_guess == NULL)
    Error("a child element <guess> must be defined in element <setting>");
  // parse setting attributes
  setting.probability = -1.0;
  setting.ordered_guesses = false;
  for (auto a = xml_setting->FirstAttribute(); a != 0; a = a->Next()) {
    if (strcmp(a->Name(), "prob") == 0) {
      setting.probability = std::atof(a->Value());
    } else if (strcmp(a->Name(), "ordered_guesses") == 0) {
      setting.ordered_guesses = std::atoi(a->Value());
    } else {
      Error("unknown attribute '%s' in element <setting>", a->Name());
    }
  }
  if (setting.probability == -1.0)
    Error("an attribute 'prob' must be defined in element <setting>");
  // go through all guesses in <words>
  if (xml_mask) HandleMasks(setting, xml_mask);
  // go through all guesses in <guesses>
  HandleGuesses(setting, xml_guess);
}

void XmlConfig::HandleMasks(Search::Config::Setting& setting,
                            tinyxml2::XMLElement* xml_mask) {
  while (xml_mask) {
    // parse mask attributes
    std::string word, rounds, twobit_threshold;
    for (auto a = xml_mask->FirstAttribute(); a != 0; a = a->Next()) {
      if (strcmp(a->Name(), "word") == 0) {
        word = a->Value();
      } else if (strcmp(a->Name(), "rounds") == 0) {
        rounds = a->Value();
      } else if (strcmp(a->Name(), "round") == 0) {
        rounds = a->Value();
      } else if (strcmp(a->Name(), "twobit_threshold") == 0) {
        twobit_threshold = a->Value();
      } else {
        Error("unknown attribute '%s' in element <mask>", a->Name());
      }
    }
    // handle word and rounds
    if (word.length() == 0)
      Error("an attribute 'word' must be defined in element <mask>");
    Search::Config::Mask mask;
    SetWordMasksRegex(mask, word, rounds);
    if (mask.bitmask.Empty())
      Error("the attribute 'word=\"%s\"' in element <mask> is invalid",
            word.c_str());
    // handle twobit_threshold
    if (twobit_threshold.length() > 0)
      mask.twobit_threshold = std::atoi(twobit_threshold.c_str());
    else
      mask.twobit_threshold = 0;
    if (mask.twobit_threshold < 0)
      Error(
          "the attribute 'twobit_threshold' in element <mask> cannot be "
          "negative");
    // combine masks with same twobit_threshold
    bool combined = false;
    for (int i = 0; i < setting.masks.size(); i++) {
      if (setting.masks[i].twobit_threshold == mask.twobit_threshold) {
        setting.masks[i].bitmask |= mask.bitmask;
        combined = true;
        break;
      }
    }
    if (!combined) setting.masks.push_back(mask);
    xml_mask = xml_mask->NextSiblingElement("mask");
  }
}

void XmlConfig::HandleGuesses(Search::Config::Setting& setting,
                              tinyxml2::XMLElement* xml_guess) {
  while (xml_guess) {
    Search::Config::Guess guess;
    guess.choice_probability = -1.0;
    std::string condition;
    // parse guess attributes
    for (auto a = xml_guess->FirstAttribute(); a != 0; a = a->Next()) {
      if (strcmp(a->Name(), "condition") == 0) {
        condition = a->Value();
      } else if (strcmp(a->Name(), "push_stack_prob") == 0) {
        Warning("unused attribute 'push_stack_prob=\"%s\"' in element <guess>",
                a->Value());
        // guess.push_stack_probability = std::atof(a->Value());
      } else if (strcmp(a->Name(), "choice_prob") == 0) {
        guess.choice_probability = std::atof(a->Value());
      } else {
        Error("unknown attribute '%s' in element <guess>", a->Name());
      }
    }
    if (condition.length() == 0)
      Error("an attribute 'condition' must be defined in element <guess>");
    if (!guess.bc.Set(condition))
      Error("the attribute 'condition=\"%s\"' is not valid", condition.c_str());
    if (guess.choice_probability == -1.0)
      Error("an attribute 'choice_prob' must be defined in element <guess>");
    if (guess.choice_probability < 0.0 || guess.choice_probability > 1.0)
      Error("the attribute 'choice_prob' must be defined within 0.0 and 1.0");
    setting.guesses.push_back(guess);
    // if we guess on '?', we also add '3', '5', '7', 'A', 'B', 'C', 'D', 'E'
    if (guess.bc == BitCondition("?")) {
      const int index[8] = {0x3, 0x5, 0x7, 0xA, 0xB, 0xC, 0xD, 0xE};
      for (int i = 0; i < 8; i++) {
        Search::Config::Guess guess;
        guess.bc = BitCondition(index[i]);
        guess.choice_probability = Search::CHOICES[guess.bc].choice_probability;
        setting.guesses.push_back(guess);
      }
    }
    xml_guess = xml_guess->NextSiblingElement("guess");
  }
}

void XmlConfig::HandlePrintConfig(tinyxml2::XMLElement* config) {
  tinyxml2::XMLElement* flags = config->FirstChildElement("flags");
  while (flags) {
    tinyxml2::XMLElement* flag = flags->FirstChildElement("flag");
    while (flag) {
      std::string name = flag->Attribute("name") ? flag->Attribute("name") : "";
      if (name.length() == 0)
        Error("print_config: no flag name is defined (e.g. name=\"main\")");
      std::string value =
          flag->Attribute("value") ? flag->Attribute("value") : "";
      if (name.length() == 0)
        Error("print_config: no flag value is defined (e.g. value=\"0\")");
      print_config_.SetFlag(name, value.compare("0") ? true : false);
      flag = flag->NextSiblingElement("flag");
    }
    flags = flags->NextSiblingElement("flags");
  }
}

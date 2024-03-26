#include <fstream>
#include <iomanip>
#include <istream>
#include <map>

#include "crypto.h"
#include "crypto_options.h"
#include "cxxopts.hpp"
#include "logfile.h"
#include "xml_config.h"

constexpr auto version_string = "nldtool v1.0.0";

int CheckCharacteristic(XmlConfig& config) {
  Logfile logfile(config.GetOptions()["log-file"].as<std::string>());
  Characteristic characteristic = config.GenerateCharacteristic();
  int check_level = config.GetOptions()["check-char"].as<int>();
  return characteristic.CheckCharacteristic(logfile, check_level, true);
}

void ConfigSearch(XmlConfig& config) {
  Logfile logfile(config.GetOptions()["log-file"].as<std::string>());
  Characteristic characteristic = config.GenerateCharacteristic();
  if (characteristic.CheckCharacteristic(logfile, 2, true) != 0) {
    std::cout << "error: initial characteristic check failed" << std::endl;
    exit(-1);
  }
  Search search(config, characteristic, logfile,
                config.GetOptions()["stack-frequency"].as<int>(),
                config.GetOptions()["random-seed"].as<int64_t>());
  search.SetDumpTime(config.GetOptions()["dump-time"].as<int>());
  search.SetDumpCount(config.GetOptions()["dump-restarts"].as<int>());
  config.PrintSearchConfig(logfile);
  config.PrintOptions(logfile);
  search.Start();
}

int main(int argc, char* argv[]) {
  try {
    cxxopts::Options options(argv[0],
                             "Search for nonlinear differential "
                             "characteristics in cryptographic "
                             "algorithms.");
    options.add_options()                                                 //
        ("h,help",                                                        //
         "print help",                                                    //
         cxxopts::value<bool>(),                                          //
         "")                                                              //
        ("v,version",                                                     //
         "print version",                                                 //
         cxxopts::value<bool>(),                                          //
         "")                                                              //
        ("i,input-file",                                                  //
         "XML file with input characteristic and configurations",         //
         cxxopts::value<std::string>(),                                   //
         "FILE")                                                          //
        ("l,log-file",                                                    //
         "filename to write log output (in addition to stdout)",          //
         cxxopts::value<std::string>(),                                   //
         "FILE")                                                          //
        ("c,check-char",                                                  //
         "check characteristic with level L",                             //
         cxxopts::value<int>()->default_value("2")->implicit_value("2"),  //
         "L")                                                             //
        ("s,start-round",                                                 //
         "start round",                                                   //
         cxxopts::value<int>()->default_value("0"),                       //
         "N")                                                             //
        ("n,num-rounds",                                                  //
         "number of rounds",                                              //
         cxxopts::value<int>()->default_value("1"),                       //
         "N")                                                             //
        ("b,blocks",                                                      //
         "number of message blocks",                                      //
         cxxopts::value<int>()->default_value("1"),                       //
         "N")                                                             //
        ("r,rate",                                                        //
         "rate of a sponge function",                                     //
         cxxopts::value<int>()->default_value("64"),                      //
         "N");

    options.add_options("Print")                                       //
        ("P,print-config",                                             //
         "XML file with print configuration",                          //
         cxxopts::value<std::string>()->default_value(                 //
             "examples/print_config.xml"),                             //
         "FILE")                                                       //
        ("I,print-info",                                               //
         "print info at every restart and in 2 * N second intervals",  //
         cxxopts::value<int>()->default_value("1"),                    //
         "N")                                                          //
        ("C,print-char",                                               //
         "print characteristic interval in seconds",                   //
         cxxopts::value<int>()->default_value("-1"),                   //
         "N")                                                          //
        ("Z,print-steps",                                              //
         "print main or all steps",                                    //
         cxxopts::value<std::string>()->default_value("main"),         //
         "(main|all)")                                                 //
        ("p,probability",                                              //
         "compute and print the differential probability",             //
         cxxopts::value<bool>(),                                       //
         "")                                                           //
        ("m,minfree-threshold",                                        //
         "threshold to start printing minfree characteristics",        //
         cxxopts::value<int>()->default_value("200"),                  //
         "N");

    options.add_options("Search")                                 //
        ("S,search-config",                                       //
         "XML file with search configuration",                    //
         cxxopts::value<std::string>(),                           //
         "FILE")                                                  //
        ("R,random-seed",                                         //
         "random seed",                                           //
         cxxopts::value<int64_t>()->default_value("-1"),          //
         "SEED")                                                  //
        ("e,end-time",                                            //
         "end search after N seconds",                            //
         cxxopts::value<int>()->default_value("-1"),              //
         "N")                                                     //
        ("E,end-found",                                           //
         "end search once first result has been found",           //
         cxxopts::value<bool>(),                                  //
         "")                                                      //
        ("F,stack-frequency",                                     //
         "after N guesses, the whole state is put on the stack",  //
         cxxopts::value<int>()->default_value("100"),             //
         "N")                                                     //
        ("d,dump-time",                                           //
         "generate dump after N hours and quit program",          //
         cxxopts::value<int>()->default_value("-1"),              //
         "N")                                                     //
        ("D,dump-restarts",                                       //
         "generate dump after N restarts and quit program",       //
         cxxopts::value<int>()->default_value("-1"),              //
         "N");

    options.add_options("XML")                                  //
        ("w,word-size",                                         //
         "word size",                                           //
         cxxopts::value<int>()->default_value("32"),            //
         "N")                                                   //
        ("z,read-steps",                                        //
         "read main or all steps",                              //
         cxxopts::value<std::string>()->default_value("main"),  //
         "(main|all)")                                          //
        ("f,function",                                          //
         "cryptographic function to analyze",                   //
         cxxopts::value<std::string>(),                         //
         "F");

    options.parse_positional("input-file");
    AddCryptoSpecificOptions(options);

    // sort the help output so that crypto-specific options are at the end
    std::vector<std::string> general_groups = {"", "Search", "Print", "XML"};
    std::vector<std::string> sorted_groups = options.groups();
    for (auto& s : general_groups)
      sorted_groups.erase(
          std::remove(sorted_groups.begin(), sorted_groups.end(), s),
          sorted_groups.end());
    sorted_groups.insert(sorted_groups.begin(), general_groups.begin(),
                         general_groups.end());

    if (argc <= 1) {
      std::cout << options.help(sorted_groups) << std::endl;
      exit(-1);
    }
    options.parse(argc, argv);

    if (options.count("help")) {
      std::cout << options.help(sorted_groups) << std::endl;
      exit(0);
    }
    if (options.count("version")) {
      std::cout << version_string << std::endl;
      exit(0);
    }

    XmlConfig config(options);
    if (!options["input-file"].as<std::string>().empty())
      config.Load(options["input-file"].as<std::string>());
    if (options.count("probability"))
      config.GetPrintConfig().SetFlag("probability", true);

    if (options.count("check-char")) {
      int result = CheckCharacteristic(config);
      exit(result);
    }

    ConfigSearch(config);

  } catch (const cxxopts::OptionException& e) {
    std::cout << "error parsing options: " << e.what() << std::endl;
    exit(-1);
  }

  exit(-1);
}

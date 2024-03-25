#ifndef TEXT_IO_FORMAT_H_
#define TEXT_IO_FORMAT_H_

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "bitmask.h"

class Characteristic;
class PrintMatrix;
class ConditionWord;
typedef std::shared_ptr<ConditionWord> ConditionWordPtr;

class PrintConfig;

/*!
 * \brief Class responsible for the print configuration and text in- and
 * output.
 *
 * This class compiles all print options from various places and uses them to
 * generate the \see PrintMatrix when requested. It also selects the chosen \see
 * PrintVisitor for output.
 */
class TextIOFormat {
  struct PrintFlag {
    bool flag_;
    bool set_;
    std::string description_;

    PrintFlag(bool flag = false, std::string description = "")
        : flag_(flag), set_(false), description_(description) {}
  };

 public:
  static const char PRINT_MAIN_WORDS[];
  static const char PRINT_SUB_WORDS[];
  static const char PRINT_CARRY_WORDS[];
  static const char PRINT_COLOR[];
  static const char PRINT_TWOBIT_DEGREES[];
  static const char PRINT_LATEX[];
  static const char PRINT_PROBABILITY[];

  TextIOFormat();
  ~TextIOFormat();

  void SetPrintConfig(const PrintConfig& print_config);
  void InitializePrintFlags();
  void SetPrintFlag(std::string name, bool value);
  bool ContainsPrintFlag(std::string name) const;
  bool GetPrintFlag(std::string name) const;
  void PrintFlags(bool print_description, std::string intend = "") const;
  void SetPrintOnlyMainSteps();
  void SetPrintMainAndSubSteps();

  void SetDefaultChars();
  void GenerateMatrix(PrintMatrix& matrix, int matrix_rows, int matrix_columns,
                      const Characteristic& characteristic);

  ConditionWordPtr GetStep(int row, int col) const;
  void SetStep(int row, int col, ConditionWordPtr condition_word);

  bool ReadCharacteristic(std::istream& fs, Characteristic& characteristic,
                          bool read_only_main_steps);
  void WriteCharacteristic(std::ostream& fs,
                           const Characteristic& characteristic);

  void ClearConditionMask();
  void SetConditionMask(Bitmask mConditionMask);

  int GetRows() const;
  int GetCols() const;

  std::vector<std::vector<ConditionWordPtr>> FilterForMainSteps(
      bool print_carries, bool only_main_steps);
  std::map<std::string, int> GetStepNameToReferenceMap();
  void SetWriteAll(bool write_all);

 private:
  std::map<std::string, PrintFlag> print_flags_;  // flag_name, flag_definition

  unsigned char bit_condition_to_char_[16];
  uint8_t char_to_bit_condition_[256];
  unsigned char delimiter_;

  Bitmask condition_word_mask_;
  Bitmask previous_condition_word_mask_;
  PrintConfig* print_config_;

  Characteristic* previous_char_;
  std::vector<std::vector<ConditionWordPtr>> condition_word_matrix;
  std::vector<int> step_;
  bool write_all_;
};

#endif  // TEXT_IO_FORMAT_H_

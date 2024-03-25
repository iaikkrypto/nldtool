#include "text_io_format.h"

#include "crypto.h"
#include "latex_print_visitor.h"
#include "print_matrix.h"
#include "print_visitor.h"
#include "xml_config.h"

const char TextIOFormat::PRINT_MAIN_WORDS[] = "main";
const char TextIOFormat::PRINT_SUB_WORDS[] = "sub";
const char TextIOFormat::PRINT_CARRY_WORDS[] = "carry";
const char TextIOFormat::PRINT_COLOR[] = "color";
const char TextIOFormat::PRINT_TWOBIT_DEGREES[] = "twobit";
const char TextIOFormat::PRINT_LATEX[] = "latex";
const char TextIOFormat::PRINT_PROBABILITY[] = "probability";

TextIOFormat::TextIOFormat() {
  delimiter_ = ':';
  SetDefaultChars();
  InitializePrintFlags();
  previous_char_ = nullptr;
  write_all_ = true;
  print_config_ = new PrintConfig();
}

TextIOFormat::~TextIOFormat() {
  if (previous_char_) {
    delete previous_char_;
    previous_char_ = nullptr;
  }
  delete print_config_;
}

void TextIOFormat::SetPrintConfig(const PrintConfig& print_config) {
  *print_config_ = print_config;
  for (auto it = print_flags_.begin(); it != print_flags_.end(); ++it) {
    if (print_config_->ContainsFlag(it->first))
      SetPrintFlag(it->first, print_config_->GetFlag(it->first));
  }
}

void TextIOFormat::InitializePrintFlags() {
  print_flags_[PRINT_MAIN_WORDS] = PrintFlag(true, "print main words");
  print_flags_[PRINT_SUB_WORDS] = PrintFlag(false, "print sub words");
  print_flags_[PRINT_CARRY_WORDS] = PrintFlag(false, "print carry words");
  print_flags_[PRINT_COLOR] = PrintFlag(true, "make colored output");
  print_flags_[PRINT_TWOBIT_DEGREES] =
      PrintFlag(false, "print twobit degrees under each word");
  print_flags_[PRINT_LATEX] = PrintFlag(false, "generate latex source code");
  print_flags_[PRINT_PROBABILITY] =
      PrintFlag(false, "print probabilities for main words");
}

void TextIOFormat::SetPrintFlag(std::string name, bool value) {
  assert(ContainsPrintFlag(name));
  print_flags_[name].flag_ = value;
  write_all_ = true;
}

bool TextIOFormat::ContainsPrintFlag(std::string name) const {
  return print_flags_.find(name) != print_flags_.end();
}

bool TextIOFormat::GetPrintFlag(std::string name) const {
  assert(ContainsPrintFlag(name));
  return print_flags_.find(name)->second.flag_;
}

void TextIOFormat::PrintFlags(bool print_description,
                              std::string intend) const {
  for (auto it = print_flags_.begin(); it != print_flags_.end(); ++it)
    if (print_description)
      printf("%s%s = %d (%s)\n", intend.c_str(), it->first.c_str(),
             it->second.flag_, it->second.description_.c_str());
    else
      printf("%s%s = %d\n", intend.c_str(), it->first.c_str(),
             it->second.flag_);
}

void TextIOFormat::SetPrintOnlyMainSteps() {
  SetPrintFlag(PRINT_MAIN_WORDS, true);
  SetPrintFlag(PRINT_SUB_WORDS, false);
  SetPrintFlag(PRINT_CARRY_WORDS, false);
}

void TextIOFormat::SetPrintMainAndSubSteps() {
  SetPrintFlag(PRINT_MAIN_WORDS, true);
  SetPrintFlag(PRINT_SUB_WORDS, true);
  SetPrintFlag(PRINT_CARRY_WORDS, false);
}

void TextIOFormat::SetDefaultChars() {
  unsigned char chars[] = "#0u3n5x71-ABCDE?";
  for (int i = 0; i < 255; ++i) char_to_bit_condition_[i] = 255;
  for (int i = 0; i < 16; ++i) {
    bit_condition_to_char_[i] = chars[i];
    char_to_bit_condition_[bit_condition_to_char_[i]] = i;
  }
}

void TextIOFormat::ClearConditionMask() { condition_word_mask_.ClearAll(); }

void TextIOFormat::SetConditionMask(Bitmask bit_mask) {
  condition_word_mask_ = bit_mask;
}

int TextIOFormat::GetRows() const { return condition_word_matrix.size(); }

int TextIOFormat::GetCols() const {
  if (condition_word_matrix.size() == 0)
    return 0;
  else
    return condition_word_matrix[0].size();
}

ConditionWordPtr TextIOFormat::GetStep(int row, int col) const {
  return condition_word_matrix[row][col];
}

void TextIOFormat::SetStep(int row, int col, ConditionWordPtr condition_word) {
  // add rows if necessary
  if (row >= GetRows()) {
    std::vector<ConditionWordPtr> tmpRow(GetCols(), (ConditionWordPtr)0);
    condition_word_matrix.resize(row + 1, tmpRow);
  }

  // add columns if necessary
  if (col >= GetCols()) {
    for (int i = 0; i < GetRows(); i++) {
      condition_word_matrix[i].resize(col + 1, (ConditionWordPtr)0);
    }
  }

  condition_word_matrix[row][col] = condition_word;
  step_.resize(
      std::max(condition_word->GetConditionMaskIndex() + 1, (int)step_.size()));
  step_[condition_word->GetConditionMaskIndex()] =
      condition_word->GetStepNumber();
}

bool TextIOFormat::ReadCharacteristic(std::istream& fs,
                                      Characteristic& characteristic,
                                      bool read_only_main_steps) {
  int N = characteristic.GetCrypto()->GetWordSize();
  int num_words = characteristic.GetCrypto()->GetNumWords();
  int word = 0;

  std::vector<std::vector<ConditionWordPtr>> condition_word_matrix =
      FilterForMainSteps(false, read_only_main_steps);
  int matrix_rows, matrix_columns;

  matrix_rows = condition_word_matrix.size();
  matrix_columns = condition_word_matrix[0].size();

  for (int r = 0; r < matrix_rows && !fs.eof(); r++) {
    for (int c = 0; c < matrix_columns && !fs.eof(); c++) {
      if (condition_word_matrix[r][c] == 0) continue;

      word = condition_word_matrix[r][c]->GetConditionMaskIndex();

      if (word >= num_words) return true;

      if (word == -1 ||
          (read_only_main_steps && !condition_word_matrix[r][c]->IsMainWord()))
        continue;

      if (condition_word_matrix[r][c]->IsCarryWord() ||
          condition_word_matrix[r][c]->IsConstantWord())
        continue;

      fs.ignore(512, delimiter_);

      if (fs.eof()) {
        std::cerr << "error: EOF at row " << r << " and column " << c
                  << std::endl;
        return false;
      }

      for (int n = N - 1; n >= 0; n--) {
        unsigned char ch;
        uint8_t bc;

        if (!(fs >> ch)) {
          std::cerr << "error: reading BitCondition" << std::endl;
          return false;
        }

        bc = char_to_bit_condition_[ch];
        if (bc == 255) {
          std::cerr << "error: invalid BitCondition [" << ch << "]"
                    << std::endl;
          return false;
        }
        if (condition_word_matrix[r][c]->GetConditionProxy(n)->GetNumBits() ==
            1)
          condition_word_matrix[r][c]->GetConditionProxy(n)->SetCondition(
              characteristic, bc);
        // characteristic.PurgeBitCondition(Bitpos(word, n), bc);
      }

      //      WriteCharacteristic(std::cout, characteristic);
    }
    if (word >= num_words) return true;
  }
  return true;
}

// build up matrix for printing only the steps we want
std::vector<std::vector<ConditionWordPtr>> TextIOFormat::FilterForMainSteps(
    bool print_carries, bool only_main_steps) {
  std::vector<std::vector<ConditionWordPtr>> ret_val = condition_word_matrix;

  // remove empty rows
  int i = 0;
  std::vector<int> del_vec;
  for (std::vector<std::vector<ConditionWordPtr>>::iterator it_rows =
           ret_val.begin();
       it_rows != ret_val.end(); ++it_rows) {
    bool is_empty = true;
    for (std::vector<ConditionWordPtr>::iterator it_col = it_rows->begin();
         it_col != it_rows->end(); ++it_col) {
      if ((!only_main_steps) || (*it_col != 0 && (*it_col)->IsMainWord()))
        is_empty = false;
      if (!print_carries && (*it_col != 0 && ((*it_col)->IsConstantWord() ||
                                              (*it_col)->IsCarryWord()))) {
        is_empty = true;
        // FIXME: carry hack
        break;
      }
    }
    if (is_empty) {
      del_vec.push_back(i);  // ret_val.erase(it_rows);
    } else
      i++;
  }

  for (i = 0; i < del_vec.size(); i++) {
    std::vector<std::vector<ConditionWordPtr>>::iterator it_rows =
        ret_val.begin();
    ret_val.erase(it_rows + del_vec[i]);
  }
  return ret_val;
}

void TextIOFormat::WriteCharacteristic(std::ostream& fs,
                                       const Characteristic& characteristic) {
  int matrix_rows = condition_word_matrix.size();
  if (matrix_rows == 0) return;
  int matrix_columns = condition_word_matrix[0].size();

  if (!previous_char_) previous_char_ = new Characteristic(characteristic);

  PrintMatrix matrix(write_all_);
  GenerateMatrix(matrix, matrix_rows, matrix_columns, characteristic);

  std::stringstream string_stream;

  // TODO: refine this block

  bool colored = matrix.GetPrintingMode() & Printable::COLORED;
  if (matrix.GetPrintingMode() & Printable::LATEXCODE) {
    LatexPrintVisitor visitor(string_stream, matrix.GetLatexColumnOrientation(),
                              matrix.GetLatexColumnTitle(), colored);
    matrix.Accept(visitor);
  } else {
    PrintVisitor visitor(string_stream, colored);
    matrix.Accept(visitor);
  }
  // matrix.Print(string_stream, characteristic.GetCrypto()->GetWordSize());

  std::string json_string = string_stream.str();
  fs << json_string;

  *previous_char_ = characteristic;
  previous_condition_word_mask_ = condition_word_mask_;
}

// bool isCarry(ConditionWordPtr* sw) {
//  return sw->GetNumBits() > 1;
//}

void TextIOFormat::GenerateMatrix(PrintMatrix& matrix, int matrix_rows,
                                  int matrix_columns,
                                  const Characteristic& characteristic) {
  bool print_main_words_ = GetPrintFlag(PRINT_MAIN_WORDS);
  bool print_sub_words_ = GetPrintFlag(PRINT_SUB_WORDS);
  bool print_carry_words_ = GetPrintFlag(PRINT_CARRY_WORDS);
  bool print_horizontal_words_ = GetPrintFlag(PRINT_CARRY_WORDS);
  bool print_color_ = GetPrintFlag(PRINT_COLOR);
  bool print_twobit_degrees_ = GetPrintFlag(PRINT_TWOBIT_DEGREES);
  bool print_latex_ = GetPrintFlag(PRINT_LATEX);
  bool print_probability = GetPrintFlag(PRINT_PROBABILITY);

  int mcols, mrows, prob_starting_col = 0, prob_sum_col, prob_columns = 0;

  if (print_latex_)
    mcols = 1 + matrix_columns;  // step number + word
  else
    mcols =
        1 + matrix_columns + matrix_columns;  // step number + word name + word
  if (print_probability) {
    prob_starting_col = mcols + 1;
    mcols += matrix_columns + 2;  // word probabilities + sum
    prob_columns = matrix_columns +
                   2;  // To know how many columns of the mcols are probability
                       // columns It is zeros if probability is not enabled!
    prob_sum_col = mcols - 1;
  }

  mrows = matrix_rows;                              // word
  if (print_twobit_degrees_) mrows += matrix_rows;  //+ twobit degrees

  matrix.SetDimensions(mrows, mcols, prob_columns);

  int lines_per_word = 1;
  if (print_twobit_degrees_) ++lines_per_word;

  // int last_step = characteristic.GetCrypto()->GetNumWords() + 1; // set on
  // maximum value
  for (int r = 0, ri = 0, ci = 0; r < matrix_rows;
       ++r, ri += lines_per_word, ci = 0) {
    int rt = ri;

    // may add step index in first column
    int step_number = 0;
    for (int i = 0; i < matrix_columns; i++)
      if (condition_word_matrix[r][i] != 0)
        step_number = condition_word_matrix[r][i]->GetStepNumber();

    // PrintableNumber* index =
    // matrix.GetPrintableNumber(condition_word_matrix[r][0]->GetConditionMaskIndex(),
    // 4);
    PrintableNumber* index = matrix.GetPrintableNumber(step_number, 3);
    matrix.Set(ri++, ci, index);
    if (print_twobit_degrees_) matrix.Set(ri++, ci, index);
    ci++;
    ri = rt;

    if (print_probability && r == 0) {
      PrintableProbabilitySum* sum =
          matrix.GetPrintableProbabilitySum(&characteristic, 1);
      matrix.SetHeader(prob_sum_col, sum);
    }

    // add (name | word) in other columns
    for (int c = 0; c < matrix_columns; ++c) {
      ConditionWordPtr sw = condition_word_matrix[r][c];
      if (sw == NULL) {
        if (!print_latex_) ci++;
        ci++;
        continue;
      }
      // TODO: decide based on ignore list and new attribute (not only by
      // NumBits)
      if ((sw->IsMainWord() && print_main_words_) ||
          (sw->IsSubWord() && print_sub_words_) ||
          (sw->IsCarryWord() && print_carry_words_) ||
          (sw->IsHorizontalWord() && print_horizontal_words_)) {
        if (!print_latex_) {
          matrix.Set(ri++, ci,
                     matrix.GetPrintableString(sw->GetName() + ":", 2));
          if (print_twobit_degrees_)
            matrix.Set(ri++, ci, matrix.GetPrintableSpace(3));
          ci++;
          ri = rt;
        }

        matrix.Set(ri++, ci,
                   matrix.GetPrintableWord(sw, &characteristic, previous_char_,
                                           &condition_word_mask_,
                                           &previous_condition_word_mask_));
        if (print_twobit_degrees_)
          matrix.Set(ri++, ci,
                     matrix.GetPrintableTwobitDegree(sw, &characteristic,
                                                     previous_char_));
        ri = rt;
        ci++;

        if (print_probability) {
          matrix.Set(ri, prob_starting_col - 1, matrix.GetPrintableSpace(3));
          PrintableWordProbability* prob = matrix.GetPrintableWordProbability(
              sw, &characteristic, previous_char_, 1);
          matrix.Set(ri, prob_starting_col + c, prob);
          // reset column title  (little bit stupid, but easiest way to get all)
          matrix.SetHeader(
              prob_starting_col + c,
              matrix.GetPrintableString("     " + sw->GetName(), 6)
                  ->SetLevel(Printable::VISIBLE));
        }
      }
    }
  }

  // build column orientation |r|c|...|c|
  std::string orientation = "|";
  for (int i = 0; i < mcols; ++i)
    if (!i)
      orientation += "r|";
    else
      orientation += "c|";
  matrix.SetLatexColumnOrientation(orientation);

  // build latex column titles
  std::string titles;
  for (int i = 0; i < mcols; ++i)
    if (!i)
      titles = "$i$";
    else {
      titles += " $\\nabla{S_{i,";
      char buffer[12];
      snprintf(buffer, 12, "%d", i - 1);
      titles += buffer;
      titles += "}}$";
    }
  matrix.SetLatexColumnTitle(titles);

  Printable::PMode mode = Printable::NONE;

  if (print_latex_) mode |= Printable::LATEXCODE;
  if (print_color_) mode |= Printable::COLORED;

  matrix.SetPrintingMode(mode);
}

std::map<std::string, int> TextIOFormat::GetStepNameToReferenceMap() {
  std::map<std::string, int> name_to_stepref;

  int matrix_rows, matrix_columns;
  matrix_rows = condition_word_matrix.size();
  matrix_columns = condition_word_matrix[0].size();

  for (int r = 0; r < matrix_rows; r++) {
    for (int c = 0; c < matrix_columns; c++) {
      if (condition_word_matrix[r][c] != 0 &&
          condition_word_matrix[r][c]->GetConditionMaskIndex() != -1) {
        std::string key = condition_word_matrix[r][c]->GetName();
        key += std::to_string(condition_word_matrix[r][c]->GetStepNumber());
        name_to_stepref.insert(std::pair<std::string, int>(
            key, condition_word_matrix[r][c]->GetConditionMaskIndex()));
      }
    }
  }

  return name_to_stepref;
}

void TextIOFormat::SetWriteAll(bool write_all) { write_all_ = write_all; }

#ifndef PRINTABLE_H_
#define PRINTABLE_H_

#include <cstdint>
#include <iostream>
#include <vector>

#include "condition_word.h"
#include "print_visitor.h"

class Characteristic;
class Bitmask;
class Crypto;
class ConditionWord;
typedef std::shared_ptr<ConditionWord> ConditionWordPtr;

typedef unsigned char uint8_t;

/*!
 * \brief Base class for all PrintMatrix elements.
 *
 * Is friends with all PrintVisitor classes that need to access its members.
 */
class Printable {
 public:
  enum PLevel {
    INVISIBLE,  // print never
    IGNORABLE,  // print only if at least one other element in row/column is
                // visible
    VISIBLE,    // print if print_only_main = false
  };

  typedef uint8_t PMode;

  static const PMode NONE = 0;
  static const PMode COLORED = 1;    // print with ASNI escape sequences for
                                     // colors
  static const PMode LATEXCODE = 2;  // print LaTex source code, with red x, u,
                                     // n

 protected:
  Printable(int printing_width, PLevel level, bool write_all);
  virtual ~Printable();

 public:
  virtual PLevel GetLevel() const;
  virtual Printable* SetLevel(PLevel level);

  virtual void Reset();
  virtual void Accept(PrintVisitor& printVisitor) = 0;
  virtual int GetWidth();

 protected:
  bool write_all_;
  int printing_width_;
  PLevel level_;
  friend class PrintMatrix;
  friend class PrintVisitor;
};

/*!
 * \brief Describes a space of certain width, mostly used for padding.
 */
class PrintableSpace : public Printable {
  int width_;

 protected:
  PrintableSpace(int width, bool write_all);

  virtual ~PrintableSpace();

 public:
  virtual void Accept(PrintVisitor& printVisitor);

  friend class PrintMatrix;
  friend class PrintVisitor;
};

/*!
 * \brief Describes text objects for output.
 */
class PrintableString : public Printable {
  std::string value_;
  uint8_t color_;

 protected:
  PrintableString(std::string value, int printing_width, uint8_t color,
                  bool write_all);

  virtual ~PrintableString();

 public:
  virtual void Accept(PrintVisitor& printVisitor);

  friend class PrintMatrix;
  friend class PrintVisitor;
};

/*!
 * \brief Describes an integer number for output.
 */
class PrintableNumber : public Printable {
  int number_;
  int width_;

 protected:
  PrintableNumber(int number, int printing_width, bool write_all);

  virtual ~PrintableNumber();

 public:
  virtual void Accept(PrintVisitor& printVisitor);

  friend class PrintMatrix;
  friend class PrintVisitor;
};

/*!
 * \brief Describes the degree of a twobit condition.
 *
 * Calculated by the \see PrintVisitor upon visiting, then outputs
 * a \see PrintableNumber or \see PrintableSpace.
 */
class PrintableTwobitDegree : public Printable {
  ConditionWordPtr word_;
  const Characteristic* characteristic_;
  const Characteristic* prev_char_;
  int N_;
  int word_index_;

 protected:
  PrintableTwobitDegree(ConditionWordPtr word,
                        const Characteristic* characteristic,
                        const Characteristic* prev_char, bool write_all);

  virtual ~PrintableTwobitDegree();

 public:
  virtual void Accept(PrintVisitor& printVisitor);

  friend class PrintMatrix;
  friend class PrintVisitor;
};

/*!
 * \brief Describes a \see ConditionWordPtr for output.
 *
 * Prints all \see BitConditions of a \see ConditionWordPtr.
 */
class PrintableWord : public Printable {
  ConditionWordPtr word_;
  const Characteristic* characteristic_;
  const Characteristic* prev_char_;
  Bitmask* condition_word_mask_;
  Bitmask* prev_condition_word_mask_;
  int N_;
  int word_index_;

 protected:
  PrintableWord(ConditionWordPtr word, const Characteristic* characteristic,
                const Characteristic* prev_char, Bitmask* condition_word_mask,
                Bitmask* prev_condition_word_mask, bool write_all);

  virtual ~PrintableWord();

 public:
  virtual void Accept(PrintVisitor& printVisitor);

  friend class PrintMatrix;
  friend class PrintVisitor;
};

/*!
 * \brief Abstract base class for the probability output.
 */
class PrintableProbability : public Printable {
  int space_before_;
  int space_after_;
  uint8_t color_;

 protected:
  PrintableProbability(int space_before, int space_after, uint8_t color,
                       bool write_all);

 public:
  virtual void Accept(PrintVisitor& printVisitor);

  virtual float GetProbability() = 0;

  friend class PrintVisitor;
};

/*!
 * \brief Describes the probability of a single \see ConditionWordPtr for
 * output.
 */
class PrintableWordProbability : public PrintableProbability {
  ConditionWordPtr word_;
  const Characteristic* characteristic_;
  const Characteristic* prev_char_;
  int N_;
  int word_index_;
  bool calculated_;
  float probability_;

 protected:
  PrintableWordProbability(ConditionWordPtr word,
                           const Characteristic* characteristic,
                           const Characteristic* prev_char, int space_before,
                           int space_after, bool write_all);
  virtual ~PrintableWordProbability();

 public:
  virtual float GetProbability();

  friend class PrintMatrix;
};

/*!
 * \brief Describes the overall probability of the \see Characteristic for
 * output.
 */
class PrintableProbabilitySum : public PrintableProbability {
  const Characteristic* characteristic_;

 protected:
  PrintableProbabilitySum(const Characteristic* characteristic,
                          int space_before, int space_after, bool write_all);

 public:
  virtual void Accept(PrintVisitor& printVisitor);

  float GetProbability();

  friend class PrintMatrix;
  friend class PrintVisitor;
};

#endif  // PRINTABLE_H_

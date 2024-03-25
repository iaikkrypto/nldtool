#ifndef LOGFILE_H_
#define LOGFILE_H_

#include <fstream>
#include <iostream>
#include <string>

/*!
 * \brief A wrapper around a stream providing several helper functions.
 */
class Logfile {
 public:
  Logfile() : file_name_("") {}

  Logfile(std::string file_name) : file_name_(file_name) {
    if (file_name_.compare(""))
      file_stream_.open(file_name_.c_str(), std::ios::out | std::ios::app);
  }

  virtual ~Logfile() {
    if (file_name_.compare("")) file_stream_.close();
  }

  std::ostream& getStream() {
    if (file_name_.compare(""))
      return file_stream_;
    else
      return std::cout;
  }

  template <typename T>
  Logfile& operator<<(const T& s) {
    if (file_name_.compare(""))
      file_stream_ << s;
    else
      std::cout << s;
    return *this;
  }

  std::string getFileName() { return file_name_; }

  // function that takes a custom stream, and returns it
  typedef Logfile& (*MyStreamManipulator)(Logfile&);

  // take in a function with the custom signature
  Logfile& operator<<(MyStreamManipulator manip) {
    // call the function, and return it's value
    return manip(*this);
  }

  // this is the type of std::cout
  typedef std::basic_ostream<char, std::char_traits<char>> CoutType;

  // this is the function signature of std::endl
  typedef CoutType& (*StandardEndLine)(CoutType&);

  // define an operator<< to take in std::endl
  Logfile& operator<<(StandardEndLine manip) {
    if (file_name_ == "")
      manip(std::cout);
    else
      manip(file_stream_);
    return *this;
  }

 private:
  std::string file_name_;
  std::fstream file_stream_;
};

#endif  // LOGFILE_H_

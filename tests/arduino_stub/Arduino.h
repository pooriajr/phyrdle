#pragma once
// Only the String/RNG surface used by Dictionary. Real AVR compilation is
// checked separately with arduino-cli and the installed Arduino core.
#include <cstdlib>
#include <string>

class String {
 public:
  String(const char *value) : value_(value) {}
  unsigned int length() const { return value_.size(); }
  char operator[](unsigned int index) const { return value_[index]; }
  const char *c_str() const { return value_.c_str(); }
 private:
  std::string value_;
};

inline long random(long upper) { return std::rand() % upper; }

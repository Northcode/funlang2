#include "mystr.hpp"

mystr make_mystr(std::string str) {
  mystr s;
  s.len = str.length();
  s.data = new char[str.length()];
  strncpy(s.data,str.c_str(),str.length());
  return s;
}

mystr make_mystr(const char* data) {
  mystr s;
  s.len = strlen(data);
  s.data = new char[s.len + 1];
  strncpy(s.data, data, s.len);
  return s;
}

#ifndef mystr_H
#define mystr_H

#include <string>
#include <iostream>
#include "string.h"
#include "stdio.h"

// @TODO: MAKE UTF8 COMPATIBLE

struct mystr {

  size_t len;
  char* data;

  friend std::ostream& operator<<(std::ostream& stream, const mystr& str) {
    for (size_t i = 0; i < str.len; i++) {
      stream << (char)str.data[i];
    }
    return stream;
  }
  
};

mystr make_mystr(std::string);
mystr make_mystr(const char* data);


#endif

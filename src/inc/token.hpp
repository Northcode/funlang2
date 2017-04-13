#ifndef TOKEN_H
#define TOKEN_H

#include "mystr.hpp"

struct token_types {
  const char* integer = "integer";
  const char* decimal = "decimal";
  const char* character = "character";
  const char* string = "string";
};

struct token {

  char* type;
  
  long filepos;

  union {
    int data_int;
    double data_decimal;
    long data_long;
    char data_char;
    mystr data_str;
  };
  
};

#endif

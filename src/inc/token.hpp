#ifndef TOKEN_H
#define TOKEN_H

#include "mystr.hpp"

enum token_types {
  Tint = 0,
  Tdecimal,
  Tchar,
  Tword,
  Tstr,

  Tpar_open,
  Tpar_close,
  Tsbrk_open,
  Tsbrk_close,
  Tbrk_open,
  Tbrk_close,

  Tshort_bin_op,
};

struct token {

  token_types type;
  
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

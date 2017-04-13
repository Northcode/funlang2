#ifndef TOKEN_H
#define TOKEN_H

#include "ratio.hpp"
#include "mystr.hpp"

enum token_type {
  t_none,

  t_nil,
  
  t_integer,
  t_decimal,
  t_ratio,

  t_chr,
  t_str,

  t_ident,

  t_true,
  t_false,

  t_def,
  t_let,
  t_fn,
  t_if,

  t_keyword,

  t_par_open,
  t_par_close,
  t_map_open,
  t_map_close,
  t_vec_open,
  t_vec_close,
  t_hash,
  
};

enum token_storage_type {
  ts_int,
  ts_rat,
  ts_dec,
  ts_long,
  ts_char,
  ts_str
};

struct token {

  token_type type;
  
  long filepos;


  token_storage_type ts;
  union {
    int data_int;
    ratio data_rat;
    double data_decimal;
    long data_long;
    char data_char;
    mystr data_str;
  };

  friend std::ostream& operator<<(std::ostream& stream, const token& tok) {

    switch (tok.ts) {
    case ts_int:
      stream << tok.data_int;
      break;
    case ts_dec:
      stream << tok.data_decimal;
      break;
    case ts_rat:
      stream << tok.data_rat.counter << "/" << tok.data_rat.divider;
      break;
    case ts_long:
      stream << tok.data_long;
      break;
    case ts_char:
      stream << tok.data_char;
      break;
    case ts_str:
      stream << tok.data_str;
      break;
    }
    return stream;
  }
  
};

#endif

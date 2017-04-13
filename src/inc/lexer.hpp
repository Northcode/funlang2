#ifndef LEXER_H
#define LEXER_H

#include <set>
#include <queue>
#include <string>

#include "string_arena.hpp"
#include "token.hpp"

struct lexer {

  std::string _input;
  std::string::iterator iter, begin, end;

  enum STATES {
    HALT,
    INIT,
    READ,
    PUSH,
    DIGIT,
    DECIMAL,
    RATIO,
    CHAR,
    STR,
    ESCAPED_STR,
    IDENT,
    KEYWORD,
    SYMBOL,
    LITERAL
  };

  STATES state;
  const std::set<char> valid_ident_chars{'_', '*', '+', '!', '-', '_', '\'', '?', '>', '<' };
  const std::set<char> valid_symbol_chars{'#','(',')','{','}','[',']'};
  const std::set<std::string> valid_literals{"true","false","nil","def","let","fn"};

  arena* _arena;
  mystr buffer;
  token_type buf_type;

  std::vector<token> tokens;

  lexer (std::string input, arena* arr);

  char& current();
  char& peek();
  char& step();

  void init_scan();

  void scan_all();
  void scan_char();

};

#endif

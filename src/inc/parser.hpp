#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include <sstream>
#include <fstream>
#include <functional>

#include "mystr.hpp"
#include "token.hpp"
#include "string_arena.hpp"

enum LEX_STATE {
  Linit = 0,
  Ldigit,
  Ldecimal,
  Lword,
  Ldone,
  Lsquote,
  Lchar,
  Ldquote,
  Lstr,
  Lesq_str,
};

enum PARSER_STATE {
  Pinit= 0,
};

struct lexer_data {
  int i;

  mystr current_str;

  token_types ttype;

  char& c() { return (char&)i; }
};

struct parser {

  LEX_STATE lstate;
  PARSER_STATE pstate;

  lexer_data ldata;

  std::vector<token> tokens;

  std::istream& inputstream;
  arena* _arena;

  parser(arena* arena, std::istream& inputstream) : inputstream(inputstream),_arena(arena) {
    lstate = Linit;
    pstate = Pinit;
    tokens = std::vector<token>();
    tokens.reserve(100); // @TODO: figure out a good number for this
  }

  void print_tokens();
  
  void scan_tokens();

  void scan_token();
  void scan_char();
  char& getc();
};

#endif

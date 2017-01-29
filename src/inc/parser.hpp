#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include <sstream>
#include <fstream>

#include "mystr.hpp"
#include "token.hpp"
#include "string_arena.hpp"

enum PARSER_STATE {

  INITIAL
  
};

struct parser {

  PARSER_STATE state;

  std::vector<token> tokens;

  std::istream& inputstream;
  arena* _arena;

  mystr current_str;

  parser(arena* arena, std::istream& inputstream) : inputstream(inputstream),_arena(arena) {
    state = INITIAL;
    tokens = std::vector<token>();
    tokens.reserve(100); // @TODO: figure out a good number for this
  }

  void print_tokens();
  
  void scan_tokens();

  void scan_token();
  char next_char();
};

#endif

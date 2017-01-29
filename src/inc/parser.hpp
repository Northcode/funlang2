#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include <sstream>
#include <fstream>

#include "mystr.hpp"
#include "token.hpp"

enum PARSER_STATE {

  INITIAL
  
};

struct parser {

  PARSER_STATE state;

  std::vector<token> tokens;

  std::istream& inputstream;

  mystr current_str;

  parser(std::istream& inputstream) : inputstream(inputstream) {
    state = INITIAL;
    tokens = std::vector<token>();
  }
  
  void scan_token();

  void scan_char();
  
  char next_char();
};

#endif

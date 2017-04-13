#include <iostream>
#include <sstream>
#include <cassert>
#include <set>

#include "mystr.hpp"
#include "string_arena.hpp"

#include "token.hpp"
#include "parser.hpp"

using namespace std;


struct lexer {

  string _input;
  string::iterator iter, begin, end;

  enum STATES {
    HALT,
    INIT,
    READ,
    PUSH,
    DIGIT,
    DECIMAL,
    STR,
    ESCAPED_STR,
    IDENT,
    CHAR,
    KEYWORD,
    SYMBOL,
  };

  STATES state;
  const set<char> valid_ident_chars{'_', '*', '+', '!', '-', '_', '\'', '?', '>', '<' };
  const set<char> valid_symbol_chars{'(',')','{','}','[',']'};

  arena* _arena;
  mystr buffer;
  
  lexer (string input, arena* arr) :
    _input(input),
    iter(_input.begin()),
    begin(_input.begin()),
    end(_input.end()),
    _arena(arr) {
    state = HALT;
  }

  char& current() {
    return *iter;
  }

  char& peek() {
    auto next = iter;
    next++;
    return *next;
  }

  char& step() {
    return *(++iter);
  }

  void scan_all() {
    init_scan();
    while (state != HALT) {
      scan_char();
    }
  }
  
  void init_scan() {
    state = INIT;
    iter = begin;
  }

  void scan_char() {

    switch (state) {
    case INIT:
      assert(iter != end);

      buffer = _arena->alloc_str(0);
      
      state = READ;
      break;
    case READ:
      assert(iter != end);

      if (isdigit(current())) {
	state = DIGIT;
      } else if (current() == '"') {
	step();
	state = STR;
      } else if (isalpha(current()) || valid_ident_chars.find(current()) != valid_ident_chars.end()) {
	state = IDENT;
      } else if (current() == '?') {
	state = CHAR;
      } else if (current() == ':') {
	step();
	state = KEYWORD;
      } else if (valid_symbol_chars.find(current()) != valid_symbol_chars.end()) {
	state = SYMBOL;
      } else {
	step();
	state = READ;
      }
      break;
    case SYMBOL:
      assert(iter != end);

      _arena->append_char(&buffer, current());
      step();
      
      if (!(valid_symbol_chars.find(current()) != valid_symbol_chars.end())) {
	state = PUSH;
      }
      
      break;
    case CHAR:
      assert(iter != end);

      step();
      _arena->append_char(&buffer, current());

      step();
      state = PUSH;

      break;
    case IDENT:
      assert(iter != end);
      {

	_arena->append_char(&buffer, current());
	step();
      
	if (!(isalnum(current()) || valid_ident_chars.find(current()) != valid_ident_chars.end())) {
	  state = PUSH;
	}
      }
      break;
    case KEYWORD:
      assert(iter != end);

      _arena->append_char(&buffer, current());
      step();

      if (isspace(current())) {
	state = PUSH;
      }
      
      break;
    case DIGIT:
      assert(iter != end);

      _arena->append_char(&buffer, current());
      step();

      if (isdigit(current())) {

      } else if (current() == '.') {
	state = DECIMAL;
      } else {
	state = PUSH;
      }
      break;
    case DECIMAL:
      assert(iter != end);
      _arena->append_char(&buffer, current());
      step();

      if (!isdigit(current())) {
	state = PUSH;
      }

      break;
    case STR:
      assert(iter != end);

      _arena->append_char(&buffer, current());
      step();

      if (current() == '\\') {
	state = ESCAPED_STR;
      } else if (current() == '"') {
	step();
	state = PUSH;
      } 
      break;
    case ESCAPED_STR:
      assert(iter != end);
      step();

      {
	char to_append = '\0';
	switch (current()) {
	case 'n':
	  to_append = '\n';
	  break;
	case 't':
	  to_append = '\t';
	  break;
	case 'r':
	  to_append = '\r';
	  break;
	case '\\':
	  to_append = '\\';
	  break;
	case 'b':
	  to_append = '\b';
	  break;
	default:
	  to_append = current();
	  break;
	}
	_arena->append_char(&buffer, to_append);
	step();
      }

      if (current() == '"') {
	state = PUSH;
      } else {
	state = STR;
      }

      break;
    case PUSH:
      cout << "token: " << buffer << "\n";

      if (iter == end) {
	state = HALT;
      } else {
	state = INIT;
      }
      break;
    case HALT:
      return;
    }
  }
  
};


int main() {

  arena arr{};

  string input = "45 (say_hello ?c \"hello,\\n world\") {:name \"Andreas\"} (isint? 4) (str->int 5) 3.14159";

  lexer lex{input,&arr};

  lex.scan_all();

  cout << endl;

  arr.dump();

  return 0;
}

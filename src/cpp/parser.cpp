#include <functional>

#include "parser.hpp"
#include "util.hpp"

#define READ_C() p->getc(); if(p->ldata.i == -1) { p->lstate = Ldone; return; } char& c = p->ldata.c();

struct state_proc {
  const char* name;
  std::function<void(parser*)> proc;
};


void consume_init(parser* p) {
  assert(p->lstate == Linit);
  
  READ_C();
  
  p->ldata.current_str = p->_arena->alloc_str(0);

  if (isdigit(c)) {
    p->lstate = Ldigit;
  } else if (isspace(c)) {
  } else if (isalpha(c)) {
    p->lstate = Lword;
  }
}

void consume_digit(parser* p) {
  assert(p->lstate == Ldigit);

  mystr& str = p->ldata.current_str;

  p->ldata.ttype = Tint; // set token type number

  p->_arena->append_char(&str, p->ldata.c());

  READ_C();

  if (isdigit(c)) {
    return; //stay in state Ldigit
  } else if (c == '.') {
    p->lstate = Ldecimal; // move to decimal state
  } else {
    p->lstate = Ldone; // dunno what c is, go to end state
  }
}

void consume_decimal(parser* p) {
  assert(p->lstate == Ldecimal);

  mystr& str = p->ldata.current_str;

  p->ldata.ttype = Tdecimal; // set token type number

  p->_arena->append_char(&str, p->ldata.c()); // append the '.'

  READ_C(); // read next

  if (isdigit(c)) {
    return;
  } else {
    p->lstate = Ldone;
  }
}

void consume_word(parser* p) {
  assert(p->lstate == Lword);

  mystr& str = p->ldata.current_str;

  p->ldata.ttype = Tstr; // set token type number

  p->_arena->append_char(&str, p->ldata.c());
  
  READ_C();

  if (isalnum(c)) {
    return;
  } else {
    p->lstate = Ldone;
  }
}

void consume_done(parser* p) {
  assert(p->lstate == Ldone);

  token t;
  t.type = p->ldata.ttype;

  switch (p->ldata.ttype) {
  case Tint: {
    mystr ntermd = p->_arena->alloc_null_term_str(p->ldata.current_str);
    
    t.data_int = atoi(ntermd.data);

    p->_arena->delete_head(&ntermd);
  } break;
  case Tdecimal: {
    mystr ntermd = p->_arena->alloc_null_term_str(p->ldata.current_str);

    t.data_decimal = atof(ntermd.data);

    p->_arena->delete_head(&ntermd);
  } break;
  case Tstr: {
    t.data_str = p->ldata.current_str;
  } break;
  };

  p->tokens.push_back(t);

  p->lstate = Linit;
}

state_proc jump_table[] = {
  { "init", &consume_init },
  { "digit", &consume_digit },
  { "decimal", &consume_decimal },
  { "word", &consume_word },
  { "done", &consume_done }
};

char& parser::getc() {
  ldata.i = inputstream.get();
  return ldata.c();
}


void parser::scan_char() {
  jump_table[lstate].proc(this);
}

void parser::scan_token() {
  while (lstate != Ldone) {
    scan_char();
  }
  scan_char(); // do the Ldone step;
}

void parser::scan_tokens() {
  while (!inputstream.eof()) {
    scan_token();
  }
}

void parser::print_tokens() {
  using std::cout;
  using std::endl;
  for (auto& tok : tokens) {
    cout << "type: " << tok.type << endl << "data: ";
    switch (tok.type) {
    case Tstr:
      cout << tok.data_str;
      break;
    case Tint:
      cout << tok.data_int;
      break;
    case Tdecimal:
      cout << tok.data_decimal;
      break;
    }
    cout << endl;
  }
}

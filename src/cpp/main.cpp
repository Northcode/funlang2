#include <iostream>
#include <sstream>
#include <cassert>
#include <set>

#include "mystr.hpp"
#include "string_arena.hpp"

#include "token.hpp"
#include "lexer.hpp"
#include "parser.hpp"


using namespace std;

int main() {

  arena arr{};

  string input = "45/100 (say_hello ?c \"hello,\\n world\") {:name \"Andreas\" :programmer? true} {:names #{\"Andreas\", \"Odd\"}} (isint? 4) (str->int 5) 3.14159";


  lexer lex{input,&arr};

  cout << "input: " << input << "\n";

  lex.scan_all();
  cout << endl;

  for (token t : lex.tokens) {
    cout << "token: " << t << "\n";
  }

  arr.dump();

  return 0;
}

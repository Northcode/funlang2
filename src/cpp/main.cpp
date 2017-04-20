#include <iostream>
#include <sstream>
#include <cassert>
#include <set>
#include <memory>

#include "mystr.hpp"
#include "string_arena.hpp"

#include "token.hpp"
#include "lexer.hpp"
#include "parser.hpp"


#include "lang_datastructs.hpp"

using namespace std;


int main() {

  arena arr{};


  pvec<int, 5> test_vec{};

  for (int i = 0; i < 35; i++) {
    test_vec = test_vec.conj(i);
  }


  cout << test_vec << endl;

  // ifstream ifinput{"sample.prog"};

  // string input{std::istreambuf_iterator<char>(ifinput), std::istreambuf_iterator<char>()};

  // lexer lex{input,&arr};

  // cout << "input: " << input << "\n";

  // lex.scan_all();
  // cout << endl;

  // for (token t : lex.tokens) {
  //   cout << "token: " << t << "\n";
  // }

  // arr.dump();

  return 0;
}

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


int main(int argc, char** argv) {

  // arena arr{};


  pvec<int, 5> test_vec{};

  for (int i = 0; i < 10000000; i++) {
    test_vec = test_vec.conj(i);
  }

  // std::vector<int> test_std_vec{};

  // for (int i = 0; i < 10000000; i++) {
  //   test_std_vec.push_back(i);
  // }

  if (argc > 1) {
    int idx = atoi(argv[1]);
    cout << "index: " << idx << " value: " << test_vec.nth(idx) << "\n";
  }


  // cout << "[";
  // for (auto i : test_std_vec) {
  //   cout << i << " ";
  // }
  // cout << "]";
  

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

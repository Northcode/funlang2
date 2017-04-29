#include <iostream>
#include <sstream>
#include <cassert>
#include <set>
#include <memory>

#include <experimental/optional>

#include "mystr.hpp"
#include "string_arena.hpp"

#include "token.hpp"
#include "lexer.hpp"
#include "parser.hpp"


#include "lang_datastructs.hpp"

using namespace std;


int main(int argc, char** argv) {

  // arena arr{};

  auto optint = std::experimental::make_optional<int>(6);
  // std::experimental::optional<int> optint{};

  optint = optint
    >>= [](int val) -> std::experimental::optional<int> { if (val > 5) { return val; } else { return {}; }};
  
  cout << optint.value_or(0) << endl;


  // pvec<int, 5> test_vec{};

  // for (int i = 0; i < 50; i++) {
  //   test_vec = test_vec.conj(i);
  // }

  // auto test_vec2 = test_vec.conj(1).conj(2).conj(3).conj(4);
  // auto maybe_test_vec3 = test_vec2.assoc(45,15);
  // auto maybe_test_vec4 = maybe_test_vec3
  //   >>= [](pvec<int, 5> vec) { return vec.assoc(3,15); };

  // cout << test_vec << "\n"
  //      << test_vec2 << "\n"
  //      << maybe_test_vec3 << "\n"
  //      << maybe_test_vec4 << "\n";

  // plist<int> numbers{};

  // auto newlist = numbers.conj(3).conj(2).conj(1);
  // auto newlist2 = newlist.conj(4).conj(5);
  // auto otherlist = newlist.pop().conj(6).conj(7);

  // cout << newlist.peek() << "\n"
  //      << newlist << "\n"
  //      << newlist2 << "\n"
  //      << otherlist << endl;

  // std::vector<int> test_std_vec{};

  // for (int i = 0; i < 10000000; i++) {
  //   test_std_vec.push_back(i);
  // }

  // if (argc > 1) {
  //   int idx = atoi(argv[1]);
  //   cout << "index: " << idx << " value: " << test_vec.nth(idx) << "\n";
  // }


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

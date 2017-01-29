#include <iostream>
#include <sstream>
#include <cassert>

#include "mystr.hpp"
#include "string_arena.hpp"

#include "token.hpp"
#include "parser.hpp"

using namespace std;

int main() {

  arena arr{};

  cout << "string: " << sizeof(std::string) << endl
       << "token:  " << sizeof(token) << endl;


  const char* input = "45 hello 'c' + \"hello,\\n world\" 3.14159";
  
  stringstream ss{input};

  parser p{&arr,ss};

  cout << "scanning: " << input << endl;

  p.scan_tokens();

  p.print_tokens();

  cout << "arena data:" << endl;
  arr.dump();

  return 0;
}

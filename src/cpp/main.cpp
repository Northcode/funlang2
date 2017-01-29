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

  mystr teststr = arr.alloc_str_from("hello, world");

  cout << "string: " << sizeof(std::string) << endl
       << "token:  " << sizeof(token) << endl;

  cout << "Message:" << teststr << endl;
  arr.append_char(&teststr,'c');
  cout << "Message:" << teststr << endl;
  arr.append_str(&teststr," testing");
  cout << "Message:" << teststr << endl;

  arr.delete_head(&teststr);

  mystr test2 = arr.alloc_str_from("this is another str");

  cout << "discarded: " << teststr << endl;
  cout << "new alloc: " << test2 << endl;
  // stringstream ss{"this is a test 2 + 4; *a = 50;"};

  // parser p{ss};

  return 0;
}

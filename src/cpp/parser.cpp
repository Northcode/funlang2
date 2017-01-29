#include "parser.hpp"

char parser::next_char() {
  return inputstream.get();
}

void parser::scan_token() {
  current_str = _arena->alloc_str(0); // empty string for appending to

  int i;
  while ((i = inputstream.get()) != -1) {
    char c = i;
    if (isspace(c)) {
      token t(Tstr);
      t.data_str = current_str;
      tokens.push_back(t);
      return;
    } else {
      _arena->append_char(&current_str, c);
    }
  }
}

void parser::scan_tokens() {
  while (!inputstream.eof()) {
    scan_token();
  }
}

void parser::print_tokens() {
  for (auto& tok : tokens) {
    std::cout << "type: " << tok.type << std::endl
	      << "data: " << tok.data_str << std::endl;
  }
}

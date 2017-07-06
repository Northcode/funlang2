#ifndef LEXER_H
#define LEXER_H

#include <queue>
#include <set>
#include <string>

#include "string_arena.hpp"
#include "token.hpp"

struct lexer
{

    const std::string _input;
    const std::string::const_iterator begin, end;
    std::string::const_iterator iter;
    int curline;

    enum STATES
    {
        HALT,
        INIT,
        READ,
        PUSH,
        DIGIT,
        DECIMAL,
        RATIO,
        CHAR,
        STR,
        ESCAPED_STR,
        IDENT,
        KEYWORD,
        SYMBOL,
        LITERAL
    };

    STATES state;
    const std::set<char> valid_ident_chars{ '_',  '*', '+', '!', '-', '_',
                                            '\'', '?', '>', '<', '=' };
    const std::set<char> valid_symbol_chars{
        '#', '(', ')', '{', '}', '[', ']'
    };
    const std::set<std::string> valid_literals{ "true", "false", "nil",
                                                "def",  "let",   "fn" };

    arena* _arena;
    mystr buffer;
    token_type buf_type;

    std::vector<token> tokens;

    lexer(std::string input, arena* arr);

    const char& current() const;
    const char& peek() const;
    const char& step();

    void init_scan();
    void error(const std::string message, int line);

    void scan_all();
    void scan_char();
};

#endif

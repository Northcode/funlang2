#ifndef PARSER_HPP
#define PARSER_HPP

#include <fstream>
#include <functional>
#include <sstream>
#include <vector>

#include "mystr.hpp"
#include "string_arena.hpp"
#include "token.hpp"

struct parser
{

    std::vector<token> tokens;

    arena* _arena;

    parser(arena* arena, std::istream& inputstream)
      : _arena(arena)
    {
        tokens = std::vector<token>();
        tokens.reserve(100); // @TODO: figure out a good number for this
    }
};

#endif

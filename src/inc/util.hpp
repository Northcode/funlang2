#ifndef UTIL_HPP
#define UTIL_HPP

#include "mystr.hpp"
#include "execinfo.h"
#include <string>

template<typename A, typename B>
struct check_types
{
    static const bool value = false;
};

template<typename A>
struct check_types<A, A>
{
    static const bool value = true;
};

int
mstoi(mystr s);
double
mstod(mystr s);

std::string get_caller();

std::string get_caller(size_t levels);

std::string get_backtrace_str();

#endif

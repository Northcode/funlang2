#ifndef UTIL_HPP
#define UTIL_HPP

#include "mystr.hpp"

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

int mstoi(mystr s);
double mstod(mystr s);

#endif

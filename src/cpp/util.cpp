#include "util.hpp"

int
mstoi(mystr s)
{
    int k = 0;
    for (size_t i = 0; i < s.len; i++) {
        k = (k << 3) + (k << 1) + (s.data[i]) - '0';
    }
    return k;
}

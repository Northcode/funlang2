#include "util.hpp"
#include <sstream>

int
mstoi(mystr s)
{
    int k = 0;
    for (size_t i = 0; i < s.len; i++) {
        k = (k << 3) + (k << 1) + (s.data[i]) - '0';
    }
    return k;
}

std::string
get_caller()
{
  size_t num_frames = 3;
  void* bt_array[num_frames];
    size_t bt_size;

    char** bt_syms;

    bt_size = backtrace(bt_array, num_frames);
    bt_syms = backtrace_symbols(bt_array, bt_size);

    auto caller = std::string(bt_syms[2]);

    free(bt_syms);
    return caller;
}

std::string
get_caller(size_t levels)
{
  size_t num_frames = levels;
  void* bt_array[num_frames];
    size_t bt_size;
char** bt_syms; bt_size = backtrace(bt_array, num_frames);
    bt_syms = backtrace_symbols(bt_array, bt_size);

    auto caller = std::string(bt_syms[levels - 1]);

    free(bt_syms);
    return caller;
}

std::string
get_backtrace_str()
{
  size_t num_frames = 14;
  void* bt_array[num_frames];
    size_t bt_size;

    char** bt_syms;

    bt_size = backtrace(bt_array, num_frames);
    bt_syms = backtrace_symbols(bt_array, bt_size);

    std::stringstream backtraces{};

    for (size_t i = 0; i < bt_size; ++i) {
      backtraces << bt_syms[i] << " | ";
    }

    free(bt_syms);
    return backtraces.str();
}

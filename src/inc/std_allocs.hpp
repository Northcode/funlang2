#ifndef STD_ALLOCS_HPP
#define STD_ALLOCS_HPP

#include "allocators.hpp"

constexpr size_t alignment = 16;

using std_alloc = typed_allocator<segregation_allocator<
  128,
  bucketize_allocator<
    freelist_allocator<stack_allocator<19500, alignment>, 0, 32, 32>,
    0,
    128,
    32>,
  malloc_allocator>>;

using verbose_std_alloc = typed_allocator<segregation_allocator<
  128,
  bucketize_allocator<
    freelist_allocator<loud_allocator<stack_allocator<8024, alignment>, 1>,
                       0,
                       32,
                       32>,
    0,
    128,
    32>,
  loud_allocator<stack_allocator<40096, alignment>, 2>>>;

using malloc_alloc = typed_allocator<malloc_allocator>;

#endif

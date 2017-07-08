#include "alloc_debug.hpp"
#include <memory>
#include <cassert>

std::unique_ptr<sptr_tracker> std_sptr_tracker;
std::unique_ptr<allocation_tracker> std_allocation_tracker;

void init_sptr_tracker() {
  assert(!std_sptr_tracker);
  std_sptr_tracker = std::make_unique<sptr_tracker>();
}

sptr_tracker* get_sptr_tracker() {
  if (!std_sptr_tracker) return nullptr;
  return std_sptr_tracker.get();
}


void
init_allocation_tracker()
{
  assert(!std_allocation_tracker);
  std_allocation_tracker = std::make_unique<allocation_tracker>();
}

allocation_tracker*
get_allocation_tracker()
{
  if (!std_sptr_tracker) return nullptr;
  return std_allocation_tracker.get();
}

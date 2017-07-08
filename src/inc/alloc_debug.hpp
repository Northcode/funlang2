#ifndef ALLOC_DEBUG_HPP
#define ALLOC_DEBUG_HPP

#include <iostream>
#include <vector>

struct sptr_allocation
{
    void* coptr;
    std::string location;

    sptr_allocation(void* ptr, std::string location)
      : coptr(ptr)
      , location(location)
    {
    }

    bool freed = false;
};

struct sptr_tracker
{
    std::vector<sptr_allocation> allocations;

    void track_allocation(void* ptr, std::string location)
    {
        allocations.emplace_back(ptr, location);
    }

    void free_allocation(void* ptr)
    {
        for (auto& allocation : allocations) {
            if (allocation.coptr == ptr) {
                allocation.freed = true;
		break;
            }
        }
    }

    void dump_stats() const
    {
        std::cout << "Smart Pointer stats:\n";
        for (auto& allocation : allocations) {
            if (!allocation.freed) {
                std::cout << allocation.coptr
                          << " allocated by: " << allocation.location
                          << " NOT FREED YET\n";
            } else {
                std::cout << allocation.coptr
                          << " allocated by: " << allocation.location
                          << " IS FREE\n";
            }
        }
    }
};

struct tracked_allocation
{
    size_t size;
    void* ptr;
    std::string location;

    tracked_allocation(size_t size, void* ptr, std::string location)
      : size(size)
      , ptr(ptr)
      , location(location)
    {
    }

    bool freed = false;
};

struct allocation_tracker
{
    std::vector<tracked_allocation> allocations;

    void track_allocation(size_t size, void* ptr, std::string location)
    {
        allocations.emplace_back(size, ptr, location);
    }

    void free_allocation(void* ptr)
    {
        for (auto& allocation : allocations) {
            if (allocation.ptr == ptr) {
                allocation.freed = true;
		break;
            }
        }
    }

    void dump_stats() const
    {
        std::cout << "ALLOCATION stats:\n";
        for (auto& allocation : allocations) {
            if (!allocation.freed) {
                std::cout << allocation.ptr
                          << " allocated by: " << allocation.location
                          << " NOT FREED YET\n";
            } else {
                std::cout << allocation.ptr
                          << " allocated by: " << allocation.location
                          << " IS FREE\n";
            }
        }
    }
};

sptr_tracker*
get_sptr_tracker();
void
init_sptr_tracker();

allocation_tracker*
get_allocation_tracker();
void
init_allocation_tracker();

#endif

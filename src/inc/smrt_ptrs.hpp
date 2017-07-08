#ifndef SMRT_PTRS_HPP
#define SMRT_PTRS_HPP

#include <functional>

#include "alloc_debug.hpp"
#include "allocators.hpp"
#include "std_allocs.hpp"
#include "util.hpp"

template<typename T, typename Allocator>
struct counted_object
{
    T* object;
    size_t uses;

    Allocator* allocator;

    counted_object(T* ptr, Allocator* alloc)
      : object(ptr)
      , uses(1)
      , allocator(alloc)
    {
    }

    ~counted_object() { allocator->template deallocate(object); }
};

template<typename U, typename Allocator>
void
sptr_deleter(void* ptr, Allocator* allocator)
{
    counted_object<U, Allocator>* cptr = (counted_object<U, Allocator>*)ptr;
    allocator->template deallocate(cptr);
}

template<typename T, typename Allocator = std_alloc>
struct sptr
{
    counted_object<T, Allocator>* counted_object_ptr;
    std::function<void(void*, Allocator*)> deleter;

    template<typename U>
    sptr(U* object, Allocator* allocator)
    {
        if (!object) {
            counted_object_ptr = nullptr;
            deleter = nullptr;
        } else {
            counted_object_ptr =
              (counted_object<T, Allocator>*)
                allocator->template allocate<counted_object<U, Allocator>>(
                  object, allocator);
            deleter = &sptr_deleter<U, Allocator>;
        }
    }

    sptr(const sptr& from)
      : counted_object_ptr(from.counted_object_ptr)
      , deleter(from.deleter)
    {
        if (counted_object_ptr) {
            assert(deleter);
            ++counted_object_ptr->uses;
        }
    }

    sptr()
      : counted_object_ptr(nullptr)
      , deleter(nullptr)
    {
    }

    ~sptr()
    {
        if (counted_object_ptr) {
            --counted_object_ptr->uses;
            if (counted_object_ptr->uses <= 0) {
                delete_object();
            }
        }
    }

    void delete_object()
    {
        assert(deleter);
        auto* alloc = counted_object_ptr->allocator;
        deleter((void*)counted_object_ptr, alloc);
        auto tracker = get_sptr_tracker();
        if (tracker)
            tracker->free_allocation(counted_object_ptr);
    }

    void operator=(const sptr& from)
    {
        if (counted_object_ptr) {
            --counted_object_ptr->uses;
            if (counted_object_ptr->uses <= 0) {
                delete_object();
            }
        }
        if (from.counted_object_ptr) {
            counted_object_ptr = from.counted_object_ptr;
            deleter = from.deleter;
            assert(deleter);
            ++counted_object_ptr->uses;
        }
    }

    T* operator->() const { return counted_object_ptr->object; }

    T& operator*() const { return *(counted_object_ptr->object); }

    operator bool() const { return counted_object_ptr != nullptr; }

    template<typename U>
    operator sptr<U, Allocator>() const
    {
        assert(counted_object_ptr);
        assert(deleter);
        sptr<U, Allocator> ns{};
        ns.counted_object_ptr =
          (counted_object<U, Allocator>*)counted_object_ptr;
        ns.deleter = deleter;
        ++ns.counted_object_ptr->uses;
        return ns;
    }

    static sptr empty() { return {}; }
};

template<typename T, typename Allocator, typename... Args>
sptr<T, Allocator>
alloc_sptr(Allocator* _allc, Args&&... args)
{
    sptr<T, Allocator> ptr{
        _allc->template allocate<T>(std::forward<Args>(args)...), _allc
    };
    auto tracker = get_sptr_tracker();
    if (tracker)
        tracker->track_allocation(ptr.counted_object_ptr, get_caller());
    return ptr;
}

#endif

#ifndef SMRT_PTRS_HPP
#define SMRT_PTRS_HPP

#include "allocators.hpp"
#include "std_allocs.hpp"

template<typename T, typename Allocator>
struct counted_object
{
    T* object;
    size_t uses;

    Allocator* allocator;

    counted_object(T* ptr, Allocator* allc)
      : object(ptr)
      , uses(1)
      , allocator(allc)
    {
        // cout << "constructing counted object\n";
    }

    ~counted_object()
    {
        // cout << "deleting counted object\n";
        allocator->template deallocate(object); 
    }
};

template<typename T, typename Allocator = std_alloc>
struct sptr
{
  counted_object<T, Allocator>* ptr;

    sptr(T* object, Allocator* allc)
    {
        // cout << "constructing sptr\n";
        ptr = allc->template allocate<counted_object<T,Allocator>>(object, allc);
    }

    sptr(const sptr& from)
      : ptr(from.ptr)
    {
      // cout << "copying sptr to pointer: " << from.ptr << " \n";
        ptr->uses++;
    }

    sptr& operator=(const sptr& from)
    {
        // cout << "copying sptr\n";
        this->ptr = from.ptr;
        ptr->uses++;
    }

    ~sptr()
    {
        // cout << "deconstructing sptr\n";
        ptr->uses--;
        if (ptr->uses <= 0) {
	    auto* alloc = ptr->allocator;
	    ptr->~counted_object<T,Allocator>();
	    alloc->template deallocate(ptr);
	}
    }

    T* operator->() { return ptr->object; }

    T& operator*() { return *(ptr->object); }
};

template<typename T, typename Allocator, typename... Args>
sptr<T, Allocator>
alloc_sptr(Allocator* _allc, Args&&... args)
{
  return { _allc->template allocate<T>(std::forward<Args>(args)...),
      _allc };
}


#endif

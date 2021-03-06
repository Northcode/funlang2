#ifndef ALLOCATORS_H
#define ALLOCATORS_H

#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

#include "alloc_debug.hpp"
#include "util.hpp"

typedef uint8_t byte;

struct block
{
    size_t size;
    void* ptr;

    friend std::ostream& operator<<(std::ostream& stream, const block& data)
    {
        stream << "BLOCK: {size:" << data.size << ", ptr:" << data.ptr << "}";
        return stream;
    }

    operator bool() { return size != 0; }

    inline static block empty()
    {
        block blk;
        blk.size = 0;
        blk.ptr = nullptr;
        return blk;
    }
};

struct malloc_allocator
{

    block allocate(size_t size)
    {
        block blk;
        blk.ptr = malloc(size);
        blk.size = size;
        return blk;
    }

    void deallocate(block blk) { free(blk.ptr); }

    bool owns(block blk) const { return true; }

    size_t good_size(size_t size) const { return size; }

    friend std::ostream& operator<<(std::ostream& stream,
                                    const malloc_allocator& data)
    {
        stream << "malloc_allocator";
        return stream;
    }
};

inline constexpr size_t
round_to_alignment(size_t basis, size_t n) noexcept
{
    return n + ((n % basis == 0) ? 0 : (basis - n % basis));
}

template<size_t MaxSize, size_t Alignment = 16>
struct stack_allocator
{

    alignas(Alignment) byte data[MaxSize];

    byte* top;

    static constexpr size_t alignment = Alignment;
    static constexpr size_t max_size = MaxSize;
    static constexpr size_t min_size = alignment;

    stack_allocator()
      : top(data)
    {
    }

    ~stack_allocator()
    {
        if (top != data) {
            // std::cout << std::distance(data, top)
            //           << " bytes unfreed in stack allocator!\n";
        }
    }

    size_t good_size(size_t size) const
    {
        return round_to_alignment(alignment, size);
    }

    block allocate(size_t size)
    {
        block result{};

        if (size == 0)
            return result;

        const auto align_len = round_to_alignment(alignment, size);

        if (align_len + top > data + MaxSize) {
	  std::cout << "STACK FULL\n";
            return result;
	}

        result.ptr = top;
        result.size = align_len;

        top += align_len;

        auto* tracker = get_allocation_tracker();
        if (tracker) {
            tracker->track_allocation(
              result.size, result.ptr, get_backtrace_str());
        }

        return result;
    }

    void deallocate(block blk)
    {
        if (!owns(blk)) {
            assert(false);
            return;
        }

        if (is_last_used_block(blk)) {
            top = static_cast<byte*>(blk.ptr);

            auto* tracker = get_allocation_tracker();
            if (tracker) {
                tracker->free_allocation(blk.ptr);
            }
        } else {
            // std::cout << "Stack free ignored, not top\n";
        }
    }

    bool is_last_used_block(const block& blk) const
    {
        return static_cast<byte*>(blk.ptr) + blk.size == top;
    }

    bool owns(block blk) const
    {
        return blk.ptr >= data && blk.ptr < data + max_size;
    }

    void dump() const
    {
        for (size_t i = 0; i < std::distance(data, top); i++) {
            std::cout << (int)data[i] << " ";
        }
        std::cout << std::endl;
    }

    friend std::ostream& operator<<(std::ostream& stream,
                                    const stack_allocator& data)
    {
        stream << "stack_allocator {\n size: " << data.max_size
               << " ,\n align: " << data.alignment << "\n }";
        return stream;
    }
};

template<size_t Size, typename T>
struct stack
{

    static constexpr size_t size = Size;

    typename std::aligned_storage<sizeof(T), alignof(T)>::type data[Size];
    size_t top = 0;

    ~stack() { reset(); }

    T& operator[](size_t pos) const { return *(T*)(data + pos); }

    void push(T obj)
    {
        if (top + 1 < size) {
            (*this)[top] = obj;
            top++;
        }
    }

    T pop()
    {
        if (top > 0) {
            top--;
            return (*this)[top];
        } else {
            std::cerr << "Popping from empty stack!\n";
            abort();
        }
    }

    const T& peek() const
    {
        if (top > 0)
            return (*this)[top - 1];
    }

    void reset()
    {
        while (top > 0) {
            top--;
            (*this)[top].~T();
        }
    }

    size_t count() const { return top; }
};

template<typename Parent, size_t MinSize, size_t MaxSize, size_t PoolSize>
struct freelist_allocator
{

    static constexpr size_t pool_size = PoolSize;

    size_t min_size = MinSize;
    size_t max_size = MaxSize;

    Parent _parent;
    stack<pool_size, block> freelist;

    freelist_allocator()
      : _parent()
      , freelist()
    {
    }
    freelist_allocator(const Parent& parent)
      : _parent(parent)
      , freelist()
    {
    }

    ~freelist_allocator()
    {
        while (freelist.count() > 0) {
            _parent.deallocate(freelist.pop());
        }
        _parent.~Parent();
    }

    void set_bounds(size_t min, size_t max)
    {
        min_size = min;
        max_size = max;
    }

    size_t good_size(size_t size) const
    {
        if (size <= max_size && size >= min_size) {
            return _parent.good_size(max_size);
        } else {
            return _parent.good_size(size);
        }
    }

    block allocate(size_t size)
    {
        if (size <= max_size && size >= min_size) {
            if (freelist.count() > 0) {
                return freelist.pop();
            } else {
                return _parent.allocate(max_size);
            }
        } else {
            return block::empty();
        }
    }

    void deallocate(block blk)
    {
        if (blk.size == max_size) {
            freelist.push(blk);
        } else {
            _parent.deallocate(blk);
        }
    }

    bool owns(block blk) const { return _parent.owns(blk); }

    void deallocate_all() { freelist.reset(); }

    friend std::ostream& operator<<(
      std::ostream& stream,
      const freelist_allocator<Parent, MinSize, MaxSize, PoolSize>& data)
    {
        stream << "freelist_allocator {\n parent: " << data._parent
               << " ,\n min: " << data.min_size << " ,\n max: " << data.max_size
               << "\n }";
        return stream;
    }
};

template<typename Primary, typename Fallback>
struct fallback_allocator
{

    Primary _primary;
    Fallback _fallback;

    fallback_allocator()
      : _primary()
      , _fallback()
    {
    }
    fallback_allocator(const Primary& prim, const Fallback& fall)
      : _primary(prim)
      , _fallback(fall)
    {
    }

    block allocate(size_t size)
    {
        block blk = _primary.allocate(size);
        if (!blk) {
            blk = _fallback.allocate(size);
        }
        return blk;
    }

    void deallocate(block blk)
    {
        if (_primary.owns(blk)) {
            _primary.deallocate(blk);
        } else {
            _fallback.deallocate(blk);
        }
    }

    bool owns(block blk) const
    {
        return _primary.owns(blk) || _fallback.owns(blk);
    }

    size_t good_size(size_t size) const
    {
        size_t s = _primary.good_size(size);
        if (s == 0) {
            s = _fallback.good_size(size);
        }
        return s;
    }

    friend std::ostream& operator<<(
      std::ostream& stream,
      const fallback_allocator<Primary, Fallback>& data)
    {
        stream << "fallback_allocator {\n primary: " << data._primary
               << " ,\n fallback: " << data._fallback << "\n }";
        return stream;
    }
};

template<size_t Threshold, typename MinAlloc, typename MaxAlloc>
struct segregation_allocator
{

    static constexpr size_t threshold = Threshold;

    MinAlloc _minalloc;
    MaxAlloc _maxalloc;

    segregation_allocator()
      : _minalloc()
      , _maxalloc()
    {
    }
    segregation_allocator(const MinAlloc& min, const MaxAlloc& max)
      : _minalloc(min)
      , _maxalloc(max)
    {
    }

    block allocate(size_t size)
    {
        if (size <= threshold) {
            return _minalloc.allocate(size);
        } else {
            return _maxalloc.allocate(size);
        }
    }

    void deallocate(block blk)
    {
        if (_minalloc.owns(blk)) {
            _minalloc.deallocate(blk);
        } else if (_maxalloc.owns(blk)) {
            _maxalloc.deallocate(blk);
        } else {
            // std::cout << "WARNING, NO ONE OWNS BLOCK: " << blk << "\n";
        }
    }

    bool owns(block blk) const
    {
      if (blk.size < threshold) {
	return _minalloc.owns(blk);
      }
      else {
        return _maxalloc.owns(blk);
      }
    }

    size_t good_size(size_t size) const
    {
        if (size < threshold) {
            return _minalloc.good_size(size);
        } else {
            return _maxalloc.good_size(size);
        }
    }

    friend std::ostream& operator<<(
      std::ostream& stream,
      const segregation_allocator<Threshold, MinAlloc, MaxAlloc>& data)
    {
        stream << "segregation_allocator {\n threshold: " << threshold
               << " ,\n minalloc: " << data._minalloc
               << " ,\n maxalloc: " << data._maxalloc << "\n }";
        return stream;
    }
};

template<typename Allocator, size_t MinSize, size_t MaxSize, size_t StepSize>
struct bucketize_allocator
{

    static constexpr size_t min_size = MinSize;
    static constexpr size_t max_size = MaxSize;
    static constexpr size_t step_size = StepSize;

    static constexpr size_t num_of_buckets = (MaxSize - MinSize + 1) / StepSize;

    std::array<Allocator, num_of_buckets> buckets;

    bucketize_allocator()
      : buckets()
    {
        for (size_t i = 0; i < num_of_buckets; i++) {
            new (&buckets[i]) Allocator();
            buckets[i].set_bounds(min_size + step_size * i,
                                  min_size + step_size * (i + 1));
        }
    }

    const Allocator* get_allocator_for(size_t size) const
    {
        assert(size >= min_size && size < max_size);

        size_t index = 0;
        for (index = 0; index < num_of_buckets; index++) {
            if (size >= buckets[index].min_size &&
                size < buckets[index].max_size) {
                break;
            }
        }

        return &buckets[index];
    }

    size_t good_size(size_t size) const
    {
        if (size >= min_size && size < max_size)
            return get_allocator_for(size)->good_size(size);
        else
            return 0;
    }

    block allocate(size_t size)
    {
        if (size >= min_size && size < max_size)
            return const_cast<Allocator*>(get_allocator_for(size))
              ->allocate(size);
        else
            return {};
    }

    bool owns(block blk) const
    {
        for (size_t i = 0; i < num_of_buckets; i++) {
            if (buckets[i].owns(blk))
                return true;
        }
        return false;
    }

    void deallocate(block blk)
    {
        for (size_t i = 0; i < num_of_buckets; i++) {
            if (buckets[i].owns(blk)) {
                buckets[i].deallocate(blk);
                return;
            }
        }
    }

    friend std::ostream& operator<<(
      std::ostream& stream,
      const bucketize_allocator<Allocator, MinSize, MaxSize, StepSize>& data)
    {
        stream << "bucketize_allocator { allocators: \n";
        for (size_t i = 0; i < num_of_buckets; i++) {
            stream << data.buckets[i] << " , \n";
        }
        stream << " }";
        return stream;
    }
};

struct empty_affix
{
};

template<typename Parent,
         typename Prefix = empty_affix,
         typename Suffix = empty_affix>
struct affix_allocator
{

    struct affixed_block : block
    {
        Prefix* prefix;
        Suffix* suffix;
    };

    Parent _parent;

    static constexpr size_t prefix_size =
      std::is_same<Prefix, empty_affix>::value ? 0 : sizeof(Prefix);
    static constexpr size_t suffix_size =
      std::is_same<Suffix, empty_affix>::value ? 0 : sizeof(Suffix);

    affix_allocator()
      : _parent()
    {
    }
    affix_allocator(const Parent& parent)
      : _parent(parent)
    {
    }

    size_t good_size(size_t size) const
    {
        return _parent.good_size(prefix_size + size + suffix_size);
    }

    bool owns(affixed_block ablk) const
    {
        block blk;
        blk.size = ablk.size + prefix_size + suffix_size;
        blk.ptr = (void*)ablk.prefix;

        return _parent.owns(blk);
    }

    affixed_block allocate(size_t size)
    {
        size_t size_to_allocate = prefix_size + size + suffix_size;
        size_t alloc_good_size = good_size(size_to_allocate);

        block blk = _parent.allocate(size_to_allocate);

        affixed_block ablk = {};

        if (!blk) {
            return ablk;
        }

        ablk.size = alloc_good_size - prefix_size - suffix_size;
        ablk.prefix = (Prefix*)blk.ptr;
        ablk.ptr = (char*)ablk.prefix + prefix_size;
        ablk.suffix = (Suffix*)((char*)ablk.ptr + ablk.size);

        return ablk;
    }

    void deallocate(affixed_block ablk)
    {
        block blk;
        blk.size = ablk.size + prefix_size + suffix_size;
        blk.ptr = (void*)ablk.prefix;

        _parent.deallocate(blk);
    }

    void dump() { _parent.dump(); }

    friend std::ostream& operator<<(
      std::ostream& stream,
      const affix_allocator<Parent, Prefix, Suffix>& data)
    {
        stream << "affix_allocator {\n prefix: " << prefix_size
               << "\n , suffix: " << suffix_size
               << "\n , parent: " << data._parent << "\n}";
        return stream;
    }
};

template<typename Parent>
struct sized_allocator
{

    using affixed_alloc = affix_allocator<Parent, size_t>;

    affixed_alloc _parent;

    sized_allocator()
      : _parent()
    {
    }
    sized_allocator(const affixed_alloc&& parent)
      : _parent(parent)
    {
    }

    void* allocate(size_t size)
    {
        typename affixed_alloc::affixed_block ablk = _parent.allocate(size);
        if (!(block)ablk) {
            return nullptr;
        }
        *ablk.prefix = ablk.size;
        return ablk.ptr;
    }

    void deallocate(void* ptr)
    {
        if (!ptr)
            return;
        typename affixed_alloc::affixed_block ablk;
        ablk.ptr = ptr;
        size_t* size = (size_t*)((char*)ptr - affixed_alloc::prefix_size);
        ablk.size = *size;
        ablk.prefix = size;

        _parent.deallocate(ablk);
    }

    size_t sizeof_alloc(void* ptr) const
    {
        return *((size_t*)((char*)ptr - affixed_alloc::prefix_size));
    }

    void dump() { _parent.dump(); }

    friend std::ostream& operator<<(std::ostream& stream,
                                    const sized_allocator<Parent>& data)
    {
        stream << "sized_allocator {\n parent: " << data._parent << " \n}";
        return stream;
    }
};

template<typename Parent>
struct typed_allocator
{

    using sized_alloc = sized_allocator<Parent>;

    sized_alloc _parent;

    typed_allocator()
      : _parent()
    {
    }
    typed_allocator(const sized_alloc&& parent)
      : _parent(parent)
    {
    }

    template<typename T, typename... Args>
    T* allocate(Args&&... args)
    {
        T* ptr = (T*)_parent.allocate(sizeof(T));
        if (!ptr) {
            return nullptr;
        }
        new (ptr) T(std::forward<Args>(args)...);
        return ptr;
    }

    template<typename T>
    void deallocate(T* ptr)
    {
        ptr->~T();
        _parent.deallocate((void*)ptr);
    }

    template<typename T>
    T* alloc_array(size_t size)
    {
        return (T*)(_parent.allocate(size * sizeof(T)));
    }

    template<typename T>
    void dealloc_array(T* arr)
    {
        size_t arr_size = _parent.sizeof_alloc(arr);

        for (size_t i = 0; i < arr_size && i < 5; i++) {
            arr[i].~T();
        }
        _parent.deallocate((void*)arr);
    }

    void dump() { _parent.dump(); }

    friend std::ostream& operator<<(std::ostream& stream,
                                    const typed_allocator<Parent>& data)
    {
        stream << "typed_allocator {\n parent: " << data._parent << "\n}";
        return stream;
    }
};

template<typename Parent, size_t TAG = 0>
struct loud_allocator
{

    Parent _parent;

    loud_allocator()
      : _parent()
    {
    }
    loud_allocator(Parent parent)
      : _parent(parent)
    {
    }

    size_t good_size(size_t size) const { return _parent.good_size(size); }

    block allocate(size_t size)
    {
        std::cout << "TAG: " << TAG << " allocating: " << size << "!\n";
        return _parent.allocate(size);
    }

    void deallocate(block blk)
    {
        std::cout << "TAG: " << TAG << " deallocating: " << blk.ptr << ", "
                  << blk.size << "!\n";
        return _parent.deallocate(blk);
    }

    bool owns(block blk) const
    {
        std::cout << "TAG: " << TAG << " checking ownership of: " << blk.ptr
                  << ", " << blk.size << " ";

        bool result = _parent.owns(blk);
        std::cout << (result ? " we own this block."
                             : " we do not own this block")
                  << "\n";
        return _parent.owns(blk);
    }

    void dump() { _parent.dump(); }

    friend std::ostream& operator<<(std::ostream& stream,
                                    const loud_allocator<Parent, TAG>& data)
    {
        stream << "loud_allocator {\n parent: " << data._parent << "\n}";
        return stream;
    }
};

#endif

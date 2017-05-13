

struct block {
  size_t size;
  void*  ptr;

  friend std::ostream& operator<<(std::ostream& stream, const block& data) {
    stream << "BLOCK: {size:" << data.size << ", ptr:" << data.ptr << "}";
    return stream;
  }

  operator bool() {
    return size != 0;
  }

  inline static block empty() {
    block blk;
    blk.size = 0;
    blk.ptr = nullptr;
    return blk;
  }
};

struct malloc_allocator {

  block allocate(size_t size) {
    block blk;
    blk.ptr = malloc(size);
    blk.size = size;
    return blk;
  }

  void deallocate(block blk) {
    free(blk.ptr);
  }

  bool owns(block blk) {
    return true;
  }

  size_t good_size(size_t size) { return size; }
};

inline constexpr size_t round_to_alignment(size_t basis, size_t n) noexcept {
  return n + ((n % basis == 0) ? 0 : (basis - n % basis));
}

template<size_t MaxSize, size_t Alignment = 16>
struct stack_allocator {

  alignas(Alignment) char data[MaxSize];

  char* top;

  static constexpr size_t max_size = MaxSize;
  static constexpr size_t alignment = Alignment;


  stack_allocator() : top(data) {}

  size_t good_size(size_t size) {
    return round_to_alignment(alignment, size);
  }

  block allocate(size_t size) {
    block result;

    if (size == 0) return result;

    const auto align_len = round_to_alignment(alignment, size);

    if (align_len + top > data + MaxSize) return result;

    result.ptr = top;
    result.size = align_len;

    top += align_len;


    return result;
  }

  void deallocate(block blk) {
    if (!owns(blk)) {
      assert(false);
      return;
    }

    std::cout << "Stack freeing block: " << blk << "\n"
	      << (void*)((char*)blk.ptr + blk.size) << "\n"
	      << "Stack top: " << (void*)top << "\n";
    
    //assert(is_last_used_block(blk) == true);
    if (is_last_used_block(blk)) {
      top = static_cast<char*>(blk.ptr);
    }
  }

  bool is_last_used_block(const block& blk) {
    return static_cast<char*>(blk.ptr) + blk.size == top;
  }

  bool owns(block blk) {
    return blk.ptr >= data && blk.ptr < data + max_size;
  }

  void dump() {
    for (size_t i = 0; i < std::distance(data, top); i++) {
      std::cout << (int)data[i] << " ";
    }
    std::cout << std::endl;
  }


};


template<size_t Size, typename T>
struct stack {

  static constexpr size_t size = Size;

  std::array<T, size> data;
  size_t top;

  ~stack() {
    reset();
  }

  void push(T obj) {
    if (top + 1 < size) {
      data[top] = obj;
      top++;
    }
  }

  T pop() {
    if (top > 0) {
      top--;
      return data[top];
    } else {
      return data[0]; // @TODO: CHANGE THIS!!!
    }
  }

  const T& peek() const {
    if (top > 0)
      return data[top - 1];
  }

  void reset() {
    while (top > 0) {
      top--;
      data[top].~T();
    }
  }

  size_t count() const {
    return top;
  }
};

template<typename Parent, size_t MinSize, size_t MaxSize, size_t PoolSize>
struct freelist_allocator {

  static constexpr size_t min_size = MinSize;
  static constexpr size_t max_size = MaxSize;
  static constexpr size_t pool_size = PoolSize;

  Parent _parent;
  stack<pool_size, block> freelist;

  freelist_allocator() : _parent(), freelist() {}
  freelist_allocator(const Parent& parent) : _parent(parent), freelist() {}

  ~freelist_allocator() {
    while (freelist.count() > 0) {
      _parent.deallocate(freelist.pop());
    }
    _parent.~Parent();
  }

  size_t good_size(size_t size) {
    if (size <= max_size && size >= min_size) {
      return _parent.good_size(max_size);
    } else {
      return _parent.good_size(size);
    }
  }
  
  block allocate(size_t size) {
    if (size <= max_size && size >= min_size) {
      if (freelist.count() > 0) {
	return freelist.pop();
      } else {
	return _parent.allocate(max_size);
      }
    } else {
      // cannot allocate, return empty block
      return block::empty();
    }
  }

  void deallocate(block blk) {
    std::cout << "block size: " << blk.size << "\n";
    if (blk.size == max_size) {
      std::cout << "using in freelist" << std::endl;
      
      freelist.push(blk);
    } else {
      _parent.deallocate(blk);
    }
  }

  void deallocate_all() {
    freelist.reset();
  }
  
};


struct empty_affix {

};

template<typename Parent, typename Prefix = empty_affix, typename Suffix = empty_affix>
struct affix_allocator {

  struct affixed_block : block {
    Prefix* prefix;
    Suffix* suffix;
  };

  Parent _parent;

  static constexpr size_t prefix_size = std::is_same<Prefix, empty_affix>::value ? 0 : sizeof(Prefix);
  static constexpr size_t suffix_size = std::is_same<Suffix, empty_affix>::value ? 0 : sizeof(Suffix);

  affix_allocator() : _parent() {}
  affix_allocator(const Parent& parent) : _parent(parent) {}

  affixed_block allocate(size_t size) {
    // cout << "Affix sizes: " << prefix_size << "," << size << "," << suffix_size << endl;
    size_t good_size = _parent.good_size(prefix_size + size + suffix_size);
    
    block blk = _parent.allocate(prefix_size + size + suffix_size);

    affixed_block ablk = {};

    if (!blk) {
      return ablk;
    }

    ablk.size = good_size - prefix_size - suffix_size;
    ablk.prefix = (Prefix*)blk.ptr;
    ablk.ptr = (char*)ablk.prefix + prefix_size;
    ablk.suffix = (Suffix*)((char*)ablk.ptr + ablk.size);

    return ablk;
  }

  void deallocate(affixed_block ablk) {
    block blk;
    blk.size = ablk.size + prefix_size + suffix_size;
    blk.ptr = (void*)ablk.prefix;

    _parent.deallocate(blk);
  }

  void dump() {
    _parent.dump();
  }
  
};

template<typename Parent>
struct sized_allocator {

  using affixed_alloc = affix_allocator<Parent, size_t>;

  affixed_alloc _parent;

  sized_allocator() : _parent() {}
  sized_allocator(const affixed_alloc&& parent) : _parent(parent) {}

  void* allocate(size_t size) {
    typename affixed_alloc::affixed_block ablk = _parent.allocate(size);
    if (!ablk) {
      return nullptr;
    }
    *ablk.prefix = ablk.size;
    return ablk.ptr;
  }

  void deallocate(void* ptr) {
    typename affixed_alloc::affixed_block ablk;
    ablk.ptr = ptr;
    size_t* size = (size_t*)((char*)ptr - affixed_alloc::prefix_size);
    ablk.size = *size;
    ablk.prefix = size;

    _parent.deallocate(ablk);
  }

  size_t sizeof_alloc(void* ptr) {
    return *((size_t*)((char*)ptr - affixed_alloc::prefix_size));
  }

  void dump() {
    _parent.dump();
  }
  
};


template<typename Parent>
struct typed_allocator {

  using sized_alloc = sized_allocator<Parent>;

  sized_alloc _parent;

  typed_allocator() : _parent() {}
  typed_allocator(const sized_alloc&& parent) : _parent(parent) {}

  template<typename T, typename ...Args>
  T* allocate(Args&&... args) {
    // cout << "making a thing of size: " << sizeof(T) << endl;
    T* ptr = (T*)_parent.allocate(sizeof(T));
    if (!ptr) {
      return nullptr;
    }
    new (ptr) T(std::forward<Args>(args)...);
    return ptr;
  }

  template<typename T>
  void deallocate(T* ptr) {
    ptr->~T();
    _parent.deallocate((void*)ptr);
  }

  template<typename T>
  T* alloc_array(size_t size) {
    // cout << "allocating array of size: " << size << " = " << size * sizeof(T) << endl;
    return (T*)(_parent.allocate(size * sizeof(T)));
  }

  template<typename T>
  void dealloc_array(T* arr) {
    // cout << sized_alloc::affixed_alloc::prefix_size << endl;
    // size_t arr_size = *(size_t*)((char*)arr - sized_alloc::affixed_alloc::prefix_size);
    size_t arr_size = _parent.sizeof_alloc(arr);
    // cout << "array size: " << arr_size << endl;
    for (size_t i = 0; i < arr_size && i < 5; i++) {
      arr[i].~T();
    }
    _parent.deallocate((void*)arr);
  }

  void dump() {
    _parent.dump();
  }
};



template<typename Parent>
struct loud_allocator {

  Parent _parent;

  loud_allocator() : _parent() {}
  loud_allocator(Parent parent) : _parent(parent) {}

  size_t good_size(size_t size) {
    return _parent.good_size(size);
  }
  
  block allocate(size_t size) {
    std::cout << "allocating: " << size << "!\n";
    return _parent.allocate(size);
  }

  void deallocate(block blk) {
    std::cout << "deallocating: " << blk.ptr << ", " << blk.size << "!\n";
    return _parent.deallocate(blk);
  }

  bool owns(block blk) {
    std::cout << "checking ownership of: " << blk.ptr << ", " << blk.size << "!\n";
    return _parent.owns(blk);
  }
  
  void dump() {
    _parent.dump();
  }
};

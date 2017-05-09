#include <iostream>
#include <sstream>
#include <cassert>
#include <set>
#include <memory>
#include <utility>

#include <experimental/optional>

#include "mystr.hpp"
#include "string_arena.hpp"

#include "token.hpp"
#include "lexer.hpp"
#include "parser.hpp"


#include "lang_datastructs.hpp"

using namespace std;


struct block {
  size_t size;
  void*  ptr;
};

struct malloc_allocator {

  block allocate(size_t size) {
    block blk{size, malloc(size)};
    return blk;
  }

  void deallocate(block blk) {
    free(blk.ptr);
  }

  bool owns(block blk) {
    return true;
  }
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
      cout << (int)data[i] << " ";
    }
    cout << endl;
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
    block blk = _parent.allocate(prefix_size + size + suffix_size);

    affixed_block ablk = {};
    ablk.size = size;
    ablk.prefix = (Prefix*)blk.ptr;
    ablk.ptr = (char*)ablk.prefix + prefix_size;
    ablk.suffix = (Suffix*)((char*)ablk.ptr + size);

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

  block allocate(size_t size) {
    cout << "allocating: " << size << "!" << endl;
    return _parent.allocate(size);
  }

  void deallocate(block blk) {
    cout << "deallocating: " << blk.ptr << ", " << blk.size << "!" << endl;
    return _parent.deallocate(blk);
  }

  bool owns(block blk) {
    cout << "checking ownership of: " << blk.ptr << ", " << blk.size << "!" << endl;
    return _parent.owns(blk);
  }
  
  void dump() {
    _parent.dump();
  }
};


struct A {
  int a,b,c;

  A(int a, int b, int c) : a(a),b(b),c(c) {
    cout << "new " << *this << endl;
  }

  ~A() {
    cout << "I am ded" << endl;
  }

  friend std::ostream& operator<<(std::ostream& stream, const A& data) {
    stream << "A: {a:" << data.a << ", b:" << data.b << ", c:" << data.c << "}";
    return stream;
  }
};


int main(int argc, char** argv) {

  // arena arr{};


  // stack_allocator<512,alignof(A)> mtest{};
  typed_allocator<loud_allocator<stack_allocator<512,alignof(A)>>> mtest{};

  A* aptr = mtest.allocate<A>(1,2,3);
  mtest.deallocate(aptr);

  A* blkb = mtest.allocate<A>(3,2,1);
  A* arr = mtest.alloc_array<A>(5);

  for (size_t i = 0; i < 5; i++) {
    new (&arr[i]) A(i*1,i*2,i*3);
  }

  cout << *aptr << endl;
  cout << *blkb << endl;
  for (size_t i = 0; i < 5; i++) {
    cout << arr[i] << endl;
  }

  // mtest.dump();

  mtest.dealloc_array(arr);
  mtest.deallocate(blkb);


  // auto optint = std::experimental::make_optional<int>(6);
  // // std::experimental::optional<int> optint{};

  // optint = optint
  //   >>= [](int val) -> std::experimental::optional<int> { if (val > 5) { return val; } else { return {}; }};
  
  // cout << optint.value_or(0) << endl;


  // pvec<int, 5> test_vec{};

  // for (int i = 0; i < 50; i++) {
  //   test_vec = test_vec.conj(i);
  // }

  // auto test_vec2 = test_vec.conj(1).conj(2).conj(3).conj(4);
  // auto maybe_test_vec3 = test_vec2.assoc(45,15);
  // auto maybe_test_vec4 = maybe_test_vec3
  //   >>= [](pvec<int, 5> vec) { return vec.assoc(3,15); };

  // cout << test_vec << "\n"
  //      << test_vec2 << "\n"
  //      << maybe_test_vec3 << "\n"
  //      << maybe_test_vec4 << "\n";

  // plist<int> numbers{};

  // auto newlist = numbers.conj(3).conj(2).conj(1);
  // auto newlist2 = newlist.conj(4).conj(5);
  // auto otherlist = newlist.pop().conj(6).conj(7);

  // cout << newlist.peek() << "\n"
  //      << newlist << "\n"
  //      << newlist2 << "\n"
  //      << otherlist << endl;

  // std::vector<int> test_std_vec{};

  // for (int i = 0; i < 10000000; i++) {
  //   test_std_vec.push_back(i);
  // }

  // if (argc > 1) {
  //   int idx = atoi(argv[1]);
  //   cout << "index: " << idx << " value: " << test_vec.nth(idx) << "\n";
  // }


  // cout << "[";
  // for (auto i : test_std_vec) {
  //   cout << i << " ";
  // }
  // cout << "]";
  

  // ifstream ifinput{"sample.prog"};

  // string input{std::istreambuf_iterator<char>(ifinput), std::istreambuf_iterator<char>()};

  // lexer lex{input,&arr};

  // cout << "input: " << input << "\n";

  // lex.scan_all();
  // cout << endl;

  // for (token t : lex.tokens) {
  //   cout << "token: " << t << "\n";
  // }

  // arr.dump();

  return 0;
}

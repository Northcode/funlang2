#include <cassert>
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <utility>

#include <experimental/optional>

#include "mystr.hpp"
#include "string_arena.hpp"

#include "lexer.hpp"
#include "parser.hpp"
#include "token.hpp"

#include "allocators.hpp"
#include "lang_datastructs.hpp"
#include "smrt_ptrs.hpp"
#include "std_allocs.hpp"

using namespace std;

struct A
{

    static constexpr bool DO_OUTPUT = false;

    int a, b, c;

    A(int a, int b, int c)
      : a(a)
      , b(b)
      , c(c)
    {
        if (DO_OUTPUT)
            cout << "new " << *this << endl;
    }

    ~A()
    {
        if (DO_OUTPUT)
            cout << "I am ded" << endl;
    }

    friend std::ostream& operator<<(std::ostream& stream, const A& data)
    {
        stream << "A: {a:" << data.a << ", b:" << data.b << ", c:" << data.c
               << "}";
        return stream;
    }
};

template<typename Allocator>
void
inc_A(sptr<A, Allocator> sp)
{
    sp->a++;
}

int main( // int argc, char** argv
  )
{

    // arena arr{};

    // stack_allocator<512,alignof(A)> mtest{};
    // using alloc =
    // typed_allocator<freelist_allocator<loud_allocator<malloc_allocator>, 10,
    // 32, 32>>;
    // typed_allocator<loud_allocator<malloc_allocator>> mtest{};

    std_alloc mtest{};

    {
        vector<sptr<A>> vec{};

        auto test = alloc_sptr<A>(&mtest, 3, 3, 3);

        {
            cout << test.ptr << endl;
            auto test2 = test;

            vec.push_back(test2);

            cout << *test2 << endl;
        }

        cout << *test << endl;
    }

    // auto optint = std::experimental::make_optional<int>(6);
    // // std::experimental::optional<int> optint{};

    // optint = optint
    //   >>= [](int val) -> std::experimental::optional<int> { if (val > 5) {
    //   return val; } else { return {}; }};

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

    // string input{std::istreambuf_iterator<char>(ifinput),
    // std::istreambuf_iterator<char>()};

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

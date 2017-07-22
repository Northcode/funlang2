#include <cassert>
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <utility>

#include <experimental/optional>

#include "affix_allocator.hpp"
#include "allocator_base.hpp"
#include "lang_datastructs.hpp"
#include "mallocator.hpp"
#include "stack_allocator.hpp"

using namespace std;


// * A struct
struct A
{

    static constexpr bool DO_OUTPUT = false;

    int a, b, c;

    A() {}

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

// * Main program
// ** Persistent Vector Range generator
template<typename T, typename Alloc>
pvec<T, Alloc>
pv_range(Alloc* _allc, T from, T to)
{
    typename pvec<T, Alloc>::tvec v{ _allc };
    for (T i = from; i < to; ++i) {
        v = v.conj(i);
    }
    return v.to_persistent();
}

// ** Main function
int main( // int argc, char** argv
  )
{

    using myalloc = alb::affix_allocator<alb::mallocator, size_t>;

    myalloc allocator;

    // pvec<A, myalloc>::tvec pvecA{ &allocator };

    // for (int i = 0; i < 10000; ++i) {
    //     pvecA = pvecA.conj({ i, i, i });
    // }

    auto rvec = pv_range<size_t>(&allocator, 0, 40);

    decltype(rvec)::key_type key = 0;

    cout << rvec << endl;

    while (true) {
        cout << "Lookup key: ";
        cin >> key;

        cout << rvec.nth(key) << endl;
    }

    // pvec<A, myalloc>::tvec tvecB{&allocator};

    // std::unique_ptr<A, alb::deleter<myalloc>> Auptr =
    // alb::make_unique<A>(allocator, 1, 2, 3);
    // std::shared_ptr<A> Asptr = alb::make_shared<A>(allocator, 2, 3, 4);

    // pvec<A, myalloc> pvec1{&allocator};

    // pvec1 = pvec1.conj({0,1,1}).conj({0,2,2});

    // auto pvec2 = pvec<A, myalloc>::create(&allocator,
    // {{0,1,1},{0,1,2},{0,1,3},{0,1,4}});

    // auto pvec3 = pvec2.pop();

    // auto tvec1 = pvec3.as_transient();

    // tvec1.conj({11,11,11}).conj({22,22,22}).conj({33,33,33});

    // tvec1.assoc(3, {55,55,55});

    // auto ptvec = tvec1.to_persistent();

    // plist<A, myalloc> plist1 = create_plist<A>(&allocator, {{1,1,1}, {2,2,2},
    // {3,3,3}});

    // plist1 = plist1.conj({0,1,1}).conj({0,2,2});

    // auto plist2 = plist1.pop();

    // auto plist3 = plist2.conj({3,4,5});

    // cout << pvec1 << endl;
    // cout << pvec2 << endl;
    // cout << pvec3 << endl;
    // cout << ptvec << endl;

    // cout << plist1 << endl;
    // cout << plist2 << endl;

    return 0;
}

#include <cassert>
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <utility>

#include <experimental/optional>

#include "allocator_base.hpp"
#include "affix_allocator.hpp"
#include "stack_allocator.hpp"
#include "mallocator.hpp"
#include "lang_datastructs.hpp"

using namespace std;

struct A
{

    static constexpr bool DO_OUTPUT = false;

    int a, b, c;

    A() {

    }

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

int main( // int argc, char** argv
  )
{

  using myalloc = alb::affix_allocator<alb::mallocator, size_t>;

  myalloc allocator;

  std::unique_ptr<A, alb::deleter<myalloc>> Auptr = alb::make_unique<A>(allocator, 1, 2, 3);
  std::shared_ptr<A> Asptr = alb::make_shared<A>(allocator, 2, 3, 4);

  pvec<A, myalloc> pvec1{&allocator};

  pvec1 = pvec1.conj({0,1,1}).conj({0,2,2});

  auto pvec2 = pvec<A, myalloc>::create(&allocator, {{0,1,1},{0,1,2},{0,1,3},{0,1,4}});

  auto pvec3 = pvec2.pop();

  auto tvec1 = pvec3.as_transient();



  plist<A, myalloc> plist1 = create_plist<A>(&allocator, {{1,1,1}, {2,2,2}, {3,3,3}});

  plist1 = plist1.conj({0,1,1}).conj({0,2,2});

  auto plist2 = plist1.pop();

  auto plist3 = plist2.conj({3,4,5});

  cout << *Auptr << endl;
  cout << *Asptr << endl;
  cout << pvec1 << endl;
  cout << pvec2 << endl;
  cout << pvec3 << endl;

  cout << plist1 << endl;
  cout << plist2 << endl;

  return 0;
}

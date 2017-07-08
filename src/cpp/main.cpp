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

template<typename Allocator>
void
inc_A(sptr<A, Allocator> sp)
{
    sp->a++;
}

int main( // int argc, char** argv
  )
{
    init_allocation_tracker();
    init_sptr_tracker();

    using s_alloc = std_alloc;

    s_alloc mtest{};

    {
	pvec<A, s_alloc> tpvec{&mtest};

	auto pvec2 = tpvec.conj({3,3,3});
	auto pvec3 = pvec2.conj({4,4,4});

	for (int i = 0; i < 200; i++) {
	    pvec2 = pvec2.conj({i,i,i});
	}

	pvec3 = pvec2.conj({50,50,50});

	for (int i = 0; i < 200; i++) {
	  pvec3 = pvec3.conj({i*2, i*2, i*2});
	}

        {
            cout << pvec2 << endl;
            cout << pvec3 << endl;
        }

        cout << tpvec << endl;
    }

    // get_sptr_tracker()->dump_stats();
    // get_allocation_tracker()->dump_stats();

	plist<A, s_alloc> tplist(&mtest);

	auto plist2 = tplist.conj({3,3,3});
	auto plist3 = tplist.conj({4,4,4});

	for (int i = 0; i < 200; i++) {
	    plist2 = plist2.conj({i,i,i});
	}

	cout << tplist << endl;
	cout << plist2 << endl;
	cout << plist3 << endl;

    return 0;
}

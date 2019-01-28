#include "type_name_pt.hpp"

/*
 *** These tests are highly dependent on platform and version ***  
*/

#if defined(__clang__)
#  define GN_CL_MS(gnu,clang,msvc) clang
#elif defined(__GNUC__) || defined(__GNUG__)
#  define GN_CL_MS(gnu,clang,msvc) gnu
#elif defined(_MSC_VER)
#  define GN_CL_MS(gnu,clang,msvc) msvc
#endif

// constexpr comparison of std::array<char,N> and char[N+1]
// *** SKIPS SPACES ***
template <size_t A, size_t B>
constexpr bool operator==( std::array<char,A> const& a, char const (&b)[B] )
{
    for (size_t ai=0, bi=0; ai!=A && (bi+1u)!=B; ++ai, ++bi)
    {
        while (a[ai]==' ') ++ai;
        while (b[bi]==' ') ++bi;
        if (a[ai] != b[bi]) return false;
    }
    return true;
}

// Non-type template arg tests

// Different Integral types give same output (not good)
static_assert( auto_name_pt<0> == "0");
static_assert( auto_name_pt<0U> == "0");
static_assert( auto_name_pt<short{}> == "0");
static_assert( auto_name_pt<long{}> == "0");

static_assert( auto_name_pt<1> == "1");
static_assert( auto_name_pt<1U> == "1");
static_assert( auto_name_pt<short{1}> == "1");
static_assert( auto_name_pt<long{1}> == "1");

// Except char c, output as 'c' for printable c, else escape sequence
static_assert( auto_name_pt<'0'> == "'0'");
static_assert( auto_name_pt<char{}> ==
GN_CL_MS (R"('\000')"
        , R"('\x00')"
        , R"('\x00')") );

enum e { a, b };
static_assert( auto_name_pt<a> == GN_CL_MS("(e)0","a","a") );
static_assert( auto_name_pt<b> == GN_CL_MS("(e)1","b","b") );
enum class E { m, n };
static_assert( auto_name_pt<E::m> == GN_CL_MS("(E)0","E::m","E::m") );
static_assert( auto_name_pt<E::n> == GN_CL_MS("(E)1","E::n","E::n") );

enum C : char { y, z };
static_assert( auto_name_pt<y> == GN_CL_MS("(C)0","y","y") );
static_assert( auto_name_pt<z> == GN_CL_MS("(C)1","z","z") );

struct ch { char c; };
static_assert( auto_name_pt<&ch::c> == "&ch::c");
constexpr ch c{};
static_assert( auto_name_pt<&c> == GN_CL_MS("(& c)", "&c", "&c") );

static_assert( type_name_pt<decltype(&ch::c)> == "char ch::*");

static_assert( type_name_pt<int> == "int");
static_assert( type_name_pt<int&> == "int&");
static_assert( type_name_pt<int*> == "int*");
static_assert( type_name_pt<int> == "int");


static_assert( type_name_pt<std::string> ==
GN_CL_MS("std::__cxx11::basic_string<char>"
       , "std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >"
       , "std::__cxx11::basic_string<char>") );

int main()
{
}
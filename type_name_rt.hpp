//    Copyright (c) 2018 Will Wray https://keybase.io/willwray
//
//   Distributed under the Boost Software License, Version 1.0.
//          (http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstring>
#include <string>
#include <string_view>
#include <typeinfo>
#include <type_traits>

#if __has_include(<cxxabi.h>)
#   include <cxxabi.h>
#   include <cstdlib>
#   include <memory>
#endif

constexpr bool CXXABI
{
#if __has_include(<cxxabi.h>)
    true
#endif
};

/*
  "type_name_rt.hpp": get type names at runtime (hence 'rt')
   ^^^^^^^^^^^^^^^^
   This header defines, in global scope (i.e. not namespace'd):
   (1) A function template type_name_str<T>() for extracting a type's name.
   (2) A variable template type_name_rt<T> initialized to the type's name.
      (also an incomplete class template IdT<T>, an implementation detail.)
   The template type parameter T is mapped to a readable name for the type.
   The work is done at runtime by what is the most standard current method;
   runtime type information (RTTI) and, on CXXABI, a demangle call
   (for compile-time alternatives see type_name_pt or type_name_ct).
   (1) type_name_str<T>()
                   Returns a std::string copy of the demangled typeid name.
                   On each call it does all the work, and cleans it all up
                   (i.e. it frees any demangle allocation once copied from).

   (2) type_name_rt<T>
                   A std::string_view global constant (a view into the
                   demangle buffer, on CXXABI, which is not ever free'd).
                   All work is done in static initialization, before main()

   Failure is signalled by an empty return value; ""
   (indicates a demangle failure as typeid is assumed fail-safe).
   Requirements:
       C++17 for string_view, constexpr-if, CTAD (unique_ptr) and __has_include
       RTTI, the compiler's runtime type information, must be enabled
   Dependencies: <string>,<string_view>,<type_traits> for std::conditional
      <typeinfo> (RTTI)
                 for typeid(T).name(), an implementation-defined name.
      <cxxabi.h> (on CXXABI platforms only - GCC, Clang, etc.)
                 for abi::__cxa_demangle(name,...)
                 to map typeid(T).name to a human readable name for T.
                 <cstdlib> for std::free, <memory> for std::unique_ptr
   E.g.
       int i;
       std::cout << type_name_rt<decltype(i)> << "\n^^^ tada!";
       --- stdout ---
       int
       ^^^ tada!
*/

// IdT<T> wraps T as template param so typeid can't decay ref or cv quals.
// An implementation detail; must be a 3-character id, any 3 chars will do.
template <typename T> struct IdT {};

namespace impl
{
// demangle<bool Free=false>( const char* name)
//
//  (1) On non-CXXABI returns name, regardless of the template parameter.
//      i.e. the function does nothing but return its parameter, a char*.
//
//  (2) On CXXABI the demangle ABI is called and the result is returned
//      with return type depending on the boolean template argument:
//    (a) char* by default (Free=false). Any demangle malloc is not free'd.
//    (b) unique_ptr<char> (Free=true) to RAII-free any malloc'd chars.
//
//     The input name should be a valid mangled name like typeid(T).name()
//     Null return value implies demangle fail (no malloc, free is harmless).
//
template <bool Free = false>
auto
demangle(char const* name) noexcept(!CXXABI)
{
  if constexpr (!CXXABI) {
      return name;         // NOP: assume already demangled if not on CXXABI
  } else {
    auto dmg = abi::__cxa_demangle(name, nullptr, nullptr, nullptr);
    if constexpr (Free)
        return std::unique_ptr<char, decltype(std::free)*>( dmg, std::free);
    else
        return dmg;
  }
}

template <bool Free>
using demangle_t = decltype(demangle<Free>(std::declval<char const*>()));

// prefix_len (constant): prefix length of demangled typeid(IdT<T>)
//   for different compilers (remove 4 chars "int>" from the length)
size_t prefix_len()
{
  static size_t const len = std::strlen(demangle<>(typeid(IdT<int>).name()))
                          - std::strlen("int>");
  return len;
}

// type_name_rt<T>()       Returns string, frees any malloc from ABI demangle
// type_name_rt<T,false>() Returns string_view, does not free demangle malloc
//
template <typename T, bool Free = true>
  // The typeid name is passed in as a (default) argument because
  // then the function body is independent of the template type arg T.
  // Compilers may emit one function, for all Ts, inlining typeid & demangle.
auto
type_name_rt( char const* tid = typeid(IdT<T>).name() ) noexcept(!CXXABI)
 ->
    std::conditional_t<Free, std::string,
                             std::string_view>
{
  if (auto dmg = demangle<Free>(tid))
  {
    size_t const p = prefix_len();
    return { &*dmg + p, std::strlen(&*dmg) - p - 1 };
  }
  return "";
}
} // namespace impl

// type_name_str<T>() Returns a std::string copy of the demangled typeid name.
//
template <typename T>
std::string
const type_name_str() { return impl::type_name_rt<T>(); }

// type_name_rt<T> Global constant; "The Demangle that Never Dangles"
//
template <typename T>
inline
std::string_view
const type_name_rt = impl::type_name_rt<T, false>();

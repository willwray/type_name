
//    Copyright (c) 2018 Will Wray https://keybase.io/willwray
//
//   Distributed under the Boost Software License, Version 1.0.
//          (http://www.boost.org/LICENSE_1_0.txt)

#pragma once 

#include "subarray.hpp"

/*
  "type_name_pt.hpp": get type names at 'preprocessor time' (hence 'pt')
   ^^^^^^^^^^^^^^^^
   This header defines, in global scope, a pair of variable templates
   (constexpr):
       type_name_pt<T> a std::array<char> containing a name for type T.
       auto_name_pt<v> a std::array<char> containing a 'name' for value v
                   (i.e. the non-type arg's value in a char-string form).
   (A future constexpr string type could replace std::array<char,N>).
   The 'name' of the template arg is extracted from the output of
   (non-standard) 'pretty function' preprocessor directives:
           __FUNCSIG__          on MSVC
           __PRETTY_FUNCTION__  on GCC, Clang, etc.
   These are constexpr-usable static C-strings of unspecified format
   that can differ between compilers and between releases of a compiler.
   I.e. this method is not portable or forward/backward compatible*.
   The 'name' of an alias is the aliased type name, often quite verbose.
   For example, the main compilers give these results for std::string:
   type_name_pt<std::string>
    on Clang: "std::__1::basic_string<char>"
    on GCC:   "std::__cxx11::basic_string<char>"
    on MSVC:  "class std::basic_string<char,struct std::char_traits<char>,
                                       class std::allocator<char> >"
    A library could provide explicit specializations for std aliases,
    and users can specialize as needed, but the need to 'register' types
    renders this lib 'pretty pointless' - may as well register all types.
    See type_name_ct for compile-time type names via TMP & registration.
    * Boost type_index is a well-supported alternative with good backward
      compatibility and multi-platform testing. It is based on the same
      preprocessor directives so has similar compromises.
*/

namespace impl
{
// Implementation note:
// There's no way to test at compile-time if __PRETTY_FUNCTION__ exists
// so test #ifdef __FUNCSIG__ instead and #else __PRETTY_FUNCTION__.

// PTTS; Preprocessor-Time Typename Size
//
template <typename T>
constexpr
auto
PTTS() // id must be 4-characters, the same length as 'PTTI' below
{
  return sizeof
  (
# if defined(__FUNCSIG__)
      __FUNCSIG__
# else
      __PRETTY_FUNCTION__
# endif
  );
}

// PTTI<T>() Returns a std::array<char,N> containing a type name for T
//           extracted from the preprocessor 'pretty function' output.
//
template <typename T>
constexpr
auto
PTTI()
{
// Changes to the output prefix are more common than suffix changes,
// so assume the suffix format is fixed and search backwards.

# if defined(__FUNCSIG__)
 // FUNCSIG example on recent MSVC:
 //
 //  PTTI<int>() ----> "auto __cdecl PTTI<int>(void)"
 //                     ^^^^^^^^^^^^^^^^^^   ^^^^^^^
 //                          prefix           suffix

    // (type arg <int> is arbitrary)
    constexpr long FS_T_prefix_size =         PTTS<int>()
                                    - sizeof(     "int>(void)");
    constexpr long FS_T_suffix_size = sizeof(        ">(void)");

    return ltl::subarray<FS_T_prefix_size,
                        -FS_T_suffix_size>(__FUNCSIG__);
# else

 // PRETTY_FUNCTION example on recent GCC:
 //
 //  PTTI<int>() ----> "constexpr auto PTTI() [with T = int]"
 //                     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^   ^
 //                                    prefix             suffix

    constexpr long PF_T_prefix_size =         PTTS<int>()
                                    - sizeof(     "int]");
    constexpr long PF_T_suffix_size = sizeof(        "]");

    return ltl::subarray<PF_T_prefix_size,
                        -PF_T_suffix_size>(__PRETTY_FUNCTION__);
# endif
}

// PTvS; Preprocessor-Time value Size
//
template <auto v>
constexpr
auto
PTvS() // id must be 4-characters, the same length as 'PTvI' below
{
  return sizeof
  (
# if defined(__FUNCSIG__)
      __FUNCSIG__
# else
      __PRETTY_FUNCTION__
# endif
  );
}

// PTvI<v>() Returns a std::array<char,N> containing a printable string for v
//           extracted from the preprocessor 'pretty function' output.
//
template <auto v>
constexpr
auto
PTvI()
{
# if defined(__FUNCSIG__)
    constexpr long FS_v_prefix_size =         PTvS<1>()
                                    - sizeof(     "1>(void)");
    constexpr long FS_v_suffix_size = sizeof(      ">(void)");

    return ltl::subarray<FS_v_prefix_size,
                        -FS_v_suffix_size>(__FUNCSIG__);
# else
    constexpr long PF_v_prefix_size =           PTvS<1>()
                                    - sizeof(       "1]");
    constexpr long PF_v_suffix_size = sizeof(        "]");

    return ltl::subarray<PF_v_prefix_size,
                        -PF_v_suffix_size>(__PRETTY_FUNCTION__);
# endif
}

} // namespace impl

template <typename T>
inline
constexpr auto type_name_pt = impl::PTTI<T>();

template <auto v>
inline
constexpr auto auto_name_pt = impl::PTvI<v>();

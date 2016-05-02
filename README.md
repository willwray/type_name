# type_name
A C++ `type_name<T>()` templated utility function for pretty-printing demangled type names.

Inspired by [Howard Hinnant's type_name code](#HH), originally posted in reponse to<br>
stackoverflow.com question [How can I see the type deduced for a template type parameter?](http://stackoverflow.com/a/18369732), August 2013.<br>

The implementation is based on C++ typeinfo, demangling the result as needed on GCC & Clang using the [Itanium ABI](http://mentorembedded.github.io/cxx-abi/) `__cxa_demangle()` function.

###Usage example
Because the type is supplied as a template parameter, `decltype(expr)` is required to query the type of an expression:
```C++
    const volatile char abc[1][2][3]{};
    std::cout << type_name<decltype(abc)>();
```
...produces output:
`char[1][2][3] const volatile`

###Disclaimer
This code experiments with techniques to optimise away unnecessary string operations. The application of these techniques to `type_name()` is illustrative. It is not recommended to use this code; use the simple, original type_name().

The aim is to improve run-time efficiency while keeping generated code small, going to constexpr extremes.

1. Avoid temporary buffer management, use of std::string, concatenation and copy operations.
2. See how far type_name can be constructed at compile-time:
  * A fully [`constexpr`](http://en.cppreference.com/w/cpp/language/constexpr) implementation is not possible in general when relying on current C++14 typeinfo.
  * Providing a template specialisation for a base type allows to avoid calls to `typeid()` and `demangle()` functions for that type (`char` in above example, for instance).<br> This could be done for all built-in types, and for user-defined types as required, in order to minimize runtime work.
2. Only call `typeid()` and `demangle` functions once per base type, caching the result via static initialisation.
  * (Howard Hinnant's function is easily changed to achieve this by adding a static variable intialised with the result.)
4. Tame the templates to produce the minimum amount of generated code.

Code tested on gcc 6.1 (c++14 default) and Clang 3.8.0 `clang -std=c++14`.<br>
(Not working on any MSVC - do not be fooled by the `_MSC_VER` conditional compile directives.)<br>
(C++14 variable template specialisations are used that fail on gcc5.)

##Design notes
Firstly, note that the type argument has to be provided as a template parameter; type deduction from a regular function argument is not sufficient because deduction cannot distinguish all possible passed types. In other words, it is not possible to wrap `decltype()` or implement it otherwise.

In principal, with the type known at compile time, a fully constexpr implementation of  `type_name<T>` should be possible, for example as a constexpr variable template whose value is some compile-time string type containing the human-readable type name.

In practice, when based on the C++ `typeid()` operator, a fully constexpr implementation is not generally possible.

This design is based on a class template with three static member functions; one to return just the base type, one to return just the type qualifiers and one to return a string concatenation of base + qualifiers:
```C++
template <typename T>
struct type_name
{
    static const char* const& base(); // base type name without qualifiers
    static constexpr auto qual();     // qualifiers; cv, array and reference
    static std::string str();         // str() returns base+qual concatenation.
};
```
Class construction is a no-op, allowing a kind of lazy evaluation.
 
The string-returning function should be avoided where possible. Stream output, for example, could be done by constructing the string and using standard string output but is much more efficiently done via the following overload:
```C++
template <typename T>
std::ostream& operator<<(std::ostream& os, type_name<T>)
{
    os << type_name<T>::base();
    if (constexpr auto qualz=type_name<T>::qual()) os << qualz;
    return os;
}
```
The `qual()` member function is constexpr; it extracts qualifiers from the template type parameter T and returns the result as a 'compile-time string' via the `auto` return type.

The `base()` member function cannot be made constexpr in general.

The C++ [`typeid()`](http://en.cppreference.com/w/cpp/language/typeid) operator is declared in the `<typeinfo>` header. 
It returns an implementation-defined name of the base type (the argument type with any const/volatile or reference qualifiers discarded). The result is returned as a c-string in some static buffer. GCC and Clang return 'mangled' names that require a further call to a 'demangle' ABI function.

<h3 id="HH">Howard Hinnant's type_name code</h2>
Slightly edited from the [Beast](https://github.com/vinniefalco/Beast) project repo [type_name.h](https://github.com/vinniefalco/Beast/blob/54f6f0ceba6e928930a306a6b062cf6b820f0ec3/beast/type_name.h)
```C++
//------------------------------------------------------------------------------ 
2 /* 
3     This file is part of Beast: https://github.com/vinniefalco/Beast 
4     Copyright 2014, Howard Hinnant <howard.hinnant@gmail.com> 
5  
6     Permission to use, copy, modify, and/or distribute this software for any 
7     purpose  with  or without fee is hereby granted, provided that the above 
8     copyright notice and this permission notice appear in all copies. 
9  
10     THE  SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES 
11     WITH  REGARD  TO  THIS  SOFTWARE  INCLUDING  ALL  IMPLIED  WARRANTIES  OF 
12     MERCHANTABILITY  AND  FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR 
13     ANY  SPECIAL ,  DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES 
14     WHATSOEVER  RESULTING  FROM  LOSS  OF USE, DATA OR PROFITS, WHETHER IN AN 
15     ACTION  OF  CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF 
16     OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
17 */ 
18 //============================================================================== 

#ifndef _MSC_VER
#   include <cxxabi.h>
#   include <memory>
#   include <cstdlib>
#endif

template <typename T>
std::string
type_name()
{
    using TR = typename std::remove_reference<T>::type;
    std::unique_ptr<char, void(*)(void*)> own (
    #ifndef _MSC_VER
        abi::__cxa_demangle (typeid(TR).name(), nullptr,
            nullptr, nullptr),
    #else
            nullptr,
    #endif
            std::free
    );
    std::string r = own != nullptr ? own.get() : typeid(TR).name();
    if (std::is_const<TR>::value)
        r += " const";
    if (std::is_volatile<TR>::value)
        r += " volatile";
    if (std::is_lvalue_reference<T>::value)
        r += "&";
    else if (std::is_rvalue_reference<T>::value)
        r += "&&";
    return r;
}

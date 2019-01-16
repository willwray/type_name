# type_name

## C++ `type_name` utilities for pretty-printing  type names.

Originally inspired by [Howard Hinnant's type_name code](#HH), posted in reponse to:   
>"[How can I see the type deduced for a template type parameter?](http://stackoverflow.com/a/18369732)"  
StackOverflow, August 2013.  

The implementation is based on C++ typeinfo (RTTI, to become constexpr in C++20).  

```c++
  typeid(T).name()
```

On GCC, Clang and compilers using the [Itanium ABI](http://mentorembedded.github.io/cxx-abi/) the result is a mangled name.  
On these platforms the name is demangled using the `abi::__cxa_demangle()` function.

## C++17 `<type_name_rt>`

1. `type_name_str<T>()`  
 Returns a `std::string` copy of the demangled `typeid` name.  
 On each call it does all the work, and cleans it all up  
 (i.e. it frees any demangle allocation once copied from).

2. `type_name_rt<T>`  
A `std::string_view` global constant (a view into the  
demangle buffer, on CXXABI, which is not ever free'd).  
All work is done in static initialization, before main()

* Failure is signaled by an empty return value; `""`  
(indicates a demangle failure as typeid is assumed failsafe).  

### Requirements

C++17 for `string_view`, constexpr-if and `__has_include`  
RTTI, the compiler's runtime type information, must be enabled.

### Dependencies

From `std`:
>`<cstring>` for `std::strlen`  
`<string>`,`<string_view>` as the return values.  
`<type_traits>` for `std::conditional`.  
`<typeinfo>` (RTTI) for typeid(T).name(), an implementation-defined name.  
`<cstdlib>` for `std::free`.  
`<memory>` for `std::unique_ptr`

Platform dependency:
>`<cxxabi.h>` for demangling (on CXXABI platforms only - GCC, Clang, etc.)  

### Usage example

Because the type is supplied as a template parameter, `decltype(expr)` is required  
to query the type of an expression:

```C++
    const volatile char abc[1][2][3]{};
    std::cout << type_name_rt<decltype(abc)>();
```

...produces output (different format is possible on different platforms):

```
  char const volatile [1][2][3]
```

## Design notes
Firstly, note that the type argument has to be provided as a template parameter; type deduction from a regular function argument is not sufficient because deduction cannot distinguish all possible passed types. In other words, it is not possible to wrap `decltype()` or implement it otherwise.

In principal, with the type known at compile time, a fully constexpr implementation of  `type_name<T>` should be possible, for example as a constexpr variable template whose value is some compile-time string type containing the human-readable type name.

In practice, when based on the C++ `typeid()` operator, a fully constexpr implementation is not generally possible.


<h3 id="HH">Howard Hinnant's C++11 type_name code</h3>

```C++
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
```

#pragma once
#include <string>
#include "compile_time_strings.h"

//===========================================================
// type_name<T>() template utility for pretty-printing demangled type names.
// Inspired by Howard Hinnant's code https://github.com/vinniefalco/Beast  utility/type_name.h

// MS Visual C++ typeinfo provides demangled names directly
// GCC, Clang typeinfo provides mangled names that require ABI call to demangle
#ifndef _MSC_VER
#   include <cxxabi.h>
#   include <memory>
#   include <cstdlib>
#endif

//-----------------------------------------------------------------
// Howard Hinnant's code
template <typename T>
std::string
hh_type_name()
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
//-----------------------------------------------------------------

// The demangler type is designed to behave like a c-string, i.e. a const char*;
// initialise with a mangled string; on success it's value will be demangled
//                                   on failure it retains the mangled string
// The implicit conversion to const char* returns a _reference_ to keep the object alive
#ifndef _MSC_VER
class demangler
{
    const char* name;
    bool demangled;
    static const char* demangle(const char* m)
    {
        const char* n {abi::__cxa_demangle(m, nullptr, nullptr, nullptr)};
        return n ? n : m;
    }
public:
    demangler(const char* m): name{demangle(m)}, demangled{name!=m} { }
    ~demangler() { if (demangled) std::free(const_cast<char*>(name)); }
    operator const char* const&() const { return name; }
};
#endif

#ifndef _MSC_VER
using basename = demangler;   // use demangling for GCC, Clang etc
#else
using basename = const char*; // no demangling for MS C++
#endif

// Tname<T>::cstr() returns a _reference_ to const char* in order to keep RAII object alive
template <typename T> struct Tname { static const char* const& cstr() { static const basename name{typeid(T).name()}; return name;} };

// Specializations for specific types prevent typeid call, and  ABI demangle call on GCC, Clang etc.
template <> struct Tname<char> { static const char* const& cstr() { static constexpr const char* name{"char"}; return name; } };

// Specializations to recursively collapse arrays
template <typename T> struct Tname<T[]> { static const char* const& cstr() { return Tname<std::remove_extent_t<T>>::cstr();} };
template <typename T, size_t N> struct Tname<T[N]> { static const char* const& cstr() { return Tname<std::remove_extent_t<T>>::cstr();} };

template <typename T> constexpr auto ref_qual = chararray("");
template <typename T> constexpr auto ref_qual<T&> = chararray("&");
template <typename T> constexpr auto ref_qual<T&&> = chararray("&&");

template <typename T> constexpr auto cv_qual = chararray("");
template <typename T> constexpr auto cv_qual<T const> = chararray(" const");
template <typename T> constexpr auto cv_qual<T volatile> = chararray(" volatile");
template <typename T> constexpr auto cv_qual<T const volatile> = chararray(" const volatile");

template <typename T> constexpr auto array_cv_qual = cv_qual<T>;
template <typename T> constexpr auto array_cv_qual<T[]> = chararray("[]") + array_cv_qual<T>;
template <typename T, size_t N> constexpr auto array_cv_qual<T[N]> = chararray("[") + i2A<N>.value() + chararray("]") + array_cv_qual<T>;

template <typename T>
struct type_name
{
	// base type name without qualifiers
    static const char* const& base() {
		return Tname<std::remove_cv_t<std::remove_reference_t<T>>>::cstr();
	}
	// qualifiers; cv, array and reference
    static constexpr auto qual() {
		return array_cv_qual<std::remove_reference_t<T>> + ref_qual<T>;
	}
	// str() returns base+qual concatenation.
	static std::string str();
	/*{
		constexpr auto qualzt = qual() + zeroterm;
		constexpr bool q{ size(qualzt) != 1 };
		return q ? std::string{ base() } +qualzt.begin() : std::string{ base() };
	}*/
};

//  stream output type_name base() and qual() separately to avoid cost of uneccessary concantenation.
template <typename T>
std::ostream& operator<<(std::ostream& os, type_name<T>)
{
    os << type_name<T>::base();
    if (constexpr auto qualz=type_name<T>::qual()) os << qualz;
    return os;
}

// Class templates & partial specializations for type qualifiers; cv, array and reference
//template <typename T> struct ref_qual      { static constexpr auto chars = chararray(""); };
//template <typename T> struct ref_qual<T&>  { static constexpr auto chars = chararray("&"); };
//template <typename T> struct ref_qual<T&&> { static constexpr auto chars = chararray("&&"); };
//
//template <typename T> struct cv_qual                   { static constexpr auto chars = chararray(""); };
//template <typename T> struct cv_qual<T const>          { static constexpr auto chars = chararray(" const"); };
//template <typename T> struct cv_qual<T volatile>       { static constexpr auto chars = chararray(" volatile"); };
//template <typename T> struct cv_qual<T const volatile> { static constexpr auto chars = chararray(" const volatile"); };
//
//template <typename T> struct array_cv_qual      { static constexpr auto chars = cv_qual<T>::chars; };
//template <typename T> struct array_cv_qual<T[]> { static constexpr auto chars = chararray("[]")
//                                                                              + array_cv_qual<T>::chars; };
//template <typename T,
//            size_t N> struct array_cv_qual<T[N]>{ static constexpr auto chars = chararray("[") + i2A<N>.value() + chararray("]")
//                                                                              + array_cv_qual<T>::chars; };

// Should be able to replace the above class templates with these variable templates


//template <typename T>
//const char* const& type_name<T>::base()
//{
//    return Tname<std::remove_cv_t<std::remove_reference_t<T>>>::cstr();
//}
//
//template <typename T>
//constexpr auto type_name<T>::qual()
//{
//    return array_cv_qual<std::remove_reference_t<T>>::chars + ref_qual<T>::chars;
//}

template <typename T>
std::string type_name<T>::str()
{
    constexpr auto qualzt = qual() + zeroterm;
    constexpr bool q {size(qualzt)!=1};
    return q ? std::string{base()} + qualzt.begin() : std::string{base()};
}

template <typename T>
void print_type_info(T&& a, const std::string& decl={})
{
    std::cout << decl << " type: " << type_name<decltype(a)>{} << ", "  << a << std::endl;
}

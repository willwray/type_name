#pragma once

#include <tuple>
#include <type_traits>
#include <typeinfo>

template <typename T, size_t N>
constexpr size_t size(const T (&)[N]) { return N; }

//===========================================================
// basic constexpr array template (c++14 std::array is not sufficiently constexpr, try c++17)
//
template <typename T, size_t N>
struct array
{
    T a[N] {};
    constexpr size_t size() const { return N; }
    constexpr T* begin() { return a; }
    constexpr T* end()  { return a+N; }
    constexpr const T* begin() const { return a; }
    constexpr const T* end() const  { return a+N; }
    constexpr T& operator[](int i) { return a[i]; }
    constexpr const T& operator[](int i) const { return a[i]; }
    constexpr void swap(size_t i, size_t j) { T tmp{a[i]}; a[i]=a[j]; a[j]=tmp; }
    explicit constexpr operator bool() const { return true; }
};

// specialization for zero-size array
//
template <typename T>
struct array<T, 0>
{
    constexpr size_t size() const { return 0; }
    constexpr const T* begin() const { return nullptr; }
    constexpr const T* end() const { return nullptr; }
    explicit constexpr operator bool() const { return false; }
};

template <typename T, size_t N>
constexpr size_t size(array<T,N>) { return N; }

// get<n>(array)
//
template< size_t I, class T, size_t N >
constexpr const T& get(const array<T,N>& a) { return a[I]; }

// stream out array 
//
template <typename T, size_t N>
std::ostream& operator<<(std::ostream& os, const array<T,N>& a)
{
    for (const auto& c : a) os << c;
    return os;
}

template <size_t N, typename T, size_t... I>
constexpr array<T,N> make_array_impl(const T* data, std::index_sequence<I...>)
{
    return {{data[I]...}};
}

// make array from data pointer
//
template <size_t N, typename T, typename Indices = std::make_index_sequence<N>>
constexpr array<T,N> make_array(const T* data)
{
    return make_array_impl<N>(data,Indices{});
}

// make constexpr array from built-in array
//
template <typename T, size_t N, typename Indices = std::make_index_sequence<N>>
constexpr array<T,N> make_array(const T (&data)[N])
{
    return make_array_impl<N,T>(data,Indices{}); 
}

// constexpr array concatenation
//
template <typename T, size_t A, size_t B, size_t... As, size_t... Bs>
constexpr array<T,A+B> concat_array_impl(const array<T,A>& a, const array<T,B>& b,
                                         std::index_sequence<As...>, std::index_sequence<Bs...>)
{
    return {{a[As]...,b[Bs]...}};
}

template <typename T, size_t A, size_t B, typename Ai=std::make_index_sequence<A>, typename Bi=std::make_index_sequence<B>>
constexpr array<T,A+B> operator+(const array<T,A>& a, const array<T,B>& b)
{
    return concat_array_impl(a,b,Ai{},Bi{});
}

// array concatenation specializations for empty array arguments
//
template <typename T, size_t A, typename Ai=std::make_index_sequence<A>>
constexpr array<T,A> operator+(const array<T,A>& a, const array<T,0>&)
{
    return a;
}

template <typename T, size_t B, typename Bi=std::make_index_sequence<B>>
constexpr array<T,B> operator+(const array<T,0>&, const array<T,B>& b)
{
    return b;
}

template <typename T>
constexpr array<T,0> operator+(const array<T,0>&, const array<T,0>&)
{
    return {};
}

//===========================================================================
// stringref is a reference to a sized chararray, assumed to be zero-terminated string-literal
// i.e. a non-owning char-array view with size a template parameter.
// (compare with string_view/span which wraps the size)
// stringlit() is a convenience 'constructor' deducing the size from its argument.
// The stringref<N>::chararray() member function returns a chararray<N-1> with null chopped off.
//
template <size_t N>
struct stringref
{
    const char (&lit)[N];
    constexpr stringref(const char (&l)[N]) : lit(l) {}
    constexpr size_t size() const { return N; }
    constexpr const char* begin() const { return lit; }
    constexpr const char* end() const { return lit+N; }
    constexpr const char& operator[](int i) const { return lit[i]; }
    constexpr array<char,N-1> chararray() const { return {make_array<N-1>(lit)}; }  // copy out without zero terminator
};

template <size_t N>
constexpr stringref<N> stringlit(const char (&l)[N]) { return {l}; }

template <size_t N>
constexpr stringref<N> stringlit(const array<char,N>& l) { static_assert(zeroterminated(l),"NZT"); return {l.a}; }

template <size_t N>
constexpr array<char,N-1> chararray(const char (&lit)[N]) { return {stringref<N>(lit).chararray()}; }

template <size_t L, size_t R>
constexpr array<char,L+R-1> operator+(const array<char,L>& l, stringref<R> r) { return l+r.chararray(); }

template <size_t L, size_t R>
constexpr array<char,L-1+R-1> operator+(stringref<L> l, stringref<R> r) { return l.chararray()+r.chararray(); }

template <size_t N>
constexpr bool operator==(stringref<N> l, stringref<N> r) { return {l.lit==r.lit}; }

template <size_t R>
std::string operator+(std::string s, stringref<R> r) { return s+r.lit; }

template <size_t N>
std::ostream& operator<<(std::ostream& os, stringref<N> s) { os << s.lit; return os; }

//===========================================================================
// chararray definitions, used as constexpr strings. No zero-terminator.
//
using nullchararray = array<char,0>;

constexpr array<char,1> zeroterm {array<char,1>{{0}}};

template <>
constexpr array<char,0> stringref<1>::chararray() const { return {array<char,0>{}}; }

template <typename A, size_t N>
constexpr bool zeroterminated(const A(&a)[N]) { return !a[N-1]; }

template <template<typename,size_t> class A, typename T, size_t N>
constexpr bool zeroterminated(const A<T,N>& a) { return !a[N-1]; }

template <template<size_t> class A, typename T, size_t N>
constexpr bool zeroterminated(const A<N>& a) { return !a[N-1]; }

//=========================================================
// constexpr array to tuple
//
template <typename Array, size_t... I>
constexpr auto a2t_impl(const Array& a, std::index_sequence<I...>) -> decltype(std::make_tuple(a[I]...))
{
    return std::make_tuple(a[I]...);
}

template <typename T, size_t N, typename Indices = std::make_index_sequence<N>>
constexpr auto a2t(const array<T,N>& a) -> decltype(a2t_impl(a,Indices{}))
{
    return a2t_impl(a,Indices{});
}

//===========================================================================
// template/constexpr variadic-char Array type.
// Currently c++14 only has the 'raw variadic' literal form for numeric literal.
// GCC and Clang have language extension for variadic string literal.
// Note that the 'literal' member is guaranteed unique so can be compared by id.
//
template <char... chars>
struct Array
{
    using type = Array<chars...>;
    using value_type = array<char,sizeof...(chars)>;
    static constexpr value_type value() { return {{chars...}}; }
    static const value_type literal; // out-of-class definition below
};

template <char... chars>
const typename Array<chars...>::value_type Array<chars...>::literal {Array<chars...>::value()};

template <>
const typename Array<>::value_type Array<>::literal;

// _t user-defined literal: decltype(12345_t) === Array<'1','2','3','4','5'>
//
template <char... chars>
constexpr Array<chars...> operator"" _t()
{
    return Array<chars...>{};
}

// _v user-defined literal: 12345_v -> constexpr array<char,5>{'1','2','3','4','5'}, an rvalue
//
template <char... chars>
constexpr typename Array<chars...>::value_type operator"" _v()
{
    return Array<chars...>::value();
}

// _id user-defined literal: 12345_id -> unique address of static array<char,5>{'1','2','3','4','5'}
//
template <char... chars>
constexpr const typename Array<chars...>::value_type* operator"" _id()
{
    return &Array<chars...>::literal;
}

template <char... Achars, char... Bchars>
constexpr Array<Achars...,Bchars...> operator+(const Array<Achars...>&, const Array<Bchars...>&)
{
    return Array<Achars...,Bchars...>{};
}

//===========================================================================
// Integer-to-Array, e.g. i2A<42> = Array<'4','2'>
//
constexpr unsigned int p10(unsigned int n) { return n ? 10*p10(n-1) : 1; }

constexpr unsigned int numdigits(size_t num, int N=1) { return num<10 ? N : numdigits(num/10,++N); }

template <size_t N, size_t... I>
constexpr auto i2A_impl(std::index_sequence<I...>) { return Array<('0'+(N/p10(numdigits(N)-1-I))%10)...>{}; }

template <size_t N, typename Indices = std::make_index_sequence<numdigits(N)>>
constexpr auto i2A = i2A_impl<N>(Indices{});

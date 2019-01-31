#include <iostream>
#include "type_name_rt.hpp"
#include "type_name_pt.hpp"

// std::array<char> output operator, for type_name_pt output
template <size_t N>
std::ostream& operator<<(std::ostream& o, std::array<char,N> const& a)
{
    for (char c : a) o.put(c);
    return o;
}

constexpr char static_var{};

int main()
{
    std::cout << "AUTO_NAME &static_var     :  " << auto_name_pt<&static_var> << '\n';
    std::cout << "AUTO_NAME char{}          :  " << auto_name_pt<char{}> << '\n';

    std::cout << '\n';

    std::cout << "TYPE_NAME char        _PT :  " << type_name_pt<char> << '\n';
    std::cout << "                      _RT :  " << type_name_rt<char> << '\n';
    std::cout << "TYPE_NAME int&        _PT :  " << type_name_pt<int&> << '\n';
    std::cout << "                      _RT :  " << type_name_rt<int&> << '\n';
    std::cout << "TYPE_NAME bool&&      _PT :  " << type_name_pt<bool&&> << '\n';
    std::cout << "                      _RT :  " << type_name_rt<bool&&> << '\n';
    std::cout << "TYPE_NAME int const&  _PT :  " << type_name_pt<int const&> << '\n';
    std::cout << "                      _RT :  " << type_name_rt<int const&> << '\n';

    std::cout << '\n';

    const volatile char abc[1][2][3]{};
    using ABC = decltype(abc);

    std::cout << "TYPE_NAME const volatile char[1][2][3]  _PT:  " << type_name_pt<ABC> << '\n';
    std::cout << "                                        _RT:  " << type_name_rt<ABC> << '\n';

    std::cout << '\n';

    std::cout << "TYPE_NAME std::string :  _PT " << type_name_pt<std::string> << '\n';
    std::cout << "                      :  _RT " << type_name_str<std::string>() << '\n';
}

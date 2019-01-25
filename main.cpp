#include <iostream>
#include "type_name_rt.hpp"
#include "type_name_pt.hpp"

int main()
{
    const volatile char abc[1][2][3]{};
    std::cout << type_name_rt<decltype(abc)> << '\n';
    std::cout << type_name_pt<decltype(abc)>.data() << std::endl;
}

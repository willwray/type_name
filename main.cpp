#include <iostream>
#include "type_name_rt.h"

int main()
{
    const volatile char abc[1][2][3]{};
	std::cout << type_name_rt<decltype(abc)> << std::endl;
}

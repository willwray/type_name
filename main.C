#include "type_name.h"

int main()
{
    const volatile char abc[1][2][3]{};
    std::cout << type_name<decltype(abc)>() << std::endl;    
}

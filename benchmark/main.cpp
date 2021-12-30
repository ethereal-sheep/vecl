#include "../include/vecl/enumerate.hpp"
#include <iostream>

auto ver  = __cplusplus;

int main()
{
    for(auto i : vecl::enumerate(5))
    {
        std::cout << i << std::endl;
    }
}
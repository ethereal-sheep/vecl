#ifndef VECL_VECTOR_HPP
#define VECL_VECTOR_HPP

namespace vecl
{
    #ifdef VECL_USE_STL
    #include <vector>
        template <typename T>
        using vector = std::pmr::vector<T>;
    #else
        template <typename T>
        class vector
        {
            /*TODO*/
        };
    #endif
}


#endif
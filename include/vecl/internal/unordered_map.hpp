#ifndef VECL_UNORDERED_MAP_HPP
#define VECL_UNORDERED_MAP_HPP

namespace vecl
{
    #ifdef VECL_USE_STL
        template <typename Key, typename Value>
        using unordered_map = std::pmr::unordered_map<Key, Value>;
    #else
        template <typename Key, typename Value>
        class unordered_map
        {
            /*TODO*/
        };
    #endif
}


#endif
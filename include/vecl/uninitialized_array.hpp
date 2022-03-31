#ifndef VECL_UNINITIALIZED_ARRAY_H
#define VECL_UNINITIALIZED_ARRAY_H

#include "config/config.h"
#include <array> // array
#include <memory> // uninitialized_move, uninitialized_copy

namespace vecl
{

    template<typename It, typename T>
    static constexpr void _uninitialized_copy(It s, It e, T* begin)
    {
        for (; s != e; ++begin, ++s)
            std::construct_at(begin, *s);
    }
    
    template<typename It, typename T>
    static constexpr void _uninitialized_move(It s, It e, T* begin)
    {
        for (; s != e; ++begin, ++s)
            std::construct_at(begin, std::move(*s));
    }

    template<typename T>
    static constexpr void _uninitialized_fill_n(T* begin, size_t n, const T& value)
    {
        for (; n--; ++begin)
            std::construct_at(begin, value);
    }

    template<typename T>
    static constexpr void _uninitialized_default_construct_n(T* begin, size_t n)
    {
        if constexpr (!std::is_trivially_default_constructible_v<T>)
        {
            for (; n > 0; ++begin, --n)
                std::construct_at(begin);
        }
    }

    /**
     * @brief array of uninitialized memory
     */
    template<typename T, size_t N>
    class uninitialized_array
    {
    public:
        constexpr uninitialized_array()
        {
            if constexpr (_is_sufficiently_trivial)
                if (std::is_constant_evaluated())
                    _uninitialized_fill_n(_storage.data(), N, T{});
        }

        /**
         * @brief Direct access to storage
         */
        constexpr const T* data() const VECL_NOEXCEPT
        {
            if constexpr (_is_sufficiently_trivial)
                return static_cast<const T*>(_storage.data());
            else
                return reinterpret_cast<const T*>(std::addressof(_storage));
        }

        /**
         * @brief Direct access to storage
         */
        constexpr T* data() VECL_NOEXCEPT
        {
            if constexpr (_is_sufficiently_trivial)
                return static_cast<T*>(_storage.data());
            else
                return reinterpret_cast<T*>(std::addressof(_storage));
        }

        constexpr size_t size() VECL_NOEXCEPT
        {
            return N;
        }

        constexpr size_t size_in_bytes() VECL_NOEXCEPT
        {
            return N * sizeof(T);
        }

    private:
        static constexpr bool _is_sufficiently_trivial =
            std::is_trivially_default_constructible_v<T> && std::is_trivially_destructible_v<T>;

        /**
         * @deprecated std::aligned_storage_t deprecated in c++23
         */
        using storage_type = std::conditional_t<
            _is_sufficiently_trivial,
            std::array<T, N>,
            std::aligned_storage_t<N * sizeof(T), alignof(T)>
        >;

        VECL_NO_UNIQUE_ADDRESS storage_type _storage;
    };
}
#endif
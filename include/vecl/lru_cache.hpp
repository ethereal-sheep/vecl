#ifndef VECL_LRU_CACHE_H
#define VECL_LRU_CACHE_H

#include "config/config.h"

#include <unordered_map> // unordered_map
#include <list> // list

namespace vecl
{
    template<typename TKey, typename T>
    class lru_cache
    {
    public:
        /**
         * @note TYPE TRAITS
         */
        using list_type = std::list<T>;
        using iterator = typename list_type::iterator;
        using map_type = std::unordered_map<TKey, iterator>;
        using key_type = TKey;
        using value_type = T;
        using reference = T&;
        using pointer = T*;
        using const_reference = const T&;
        using const_pointer = const T*;
        using size_type = size_t;

    private:
        void _elevate_to_front(iterator it) const
        {
            _list.splice(_list.begin(), _list, it);
        }

        void _evict_one()
        {
			_list.pop_back();
        }
        void _evict_n(size_t n)
        {
            while(n--) _list.pop_back();
        }
        
    public:
        /**
         * @note TYPE TRAITS
         */
        using list_type = std::list<T>;
		using iterator = typename list_type::iterator;
        using map_type = std::unordered_map<TKey, iterator>;
		using key_type = TKey;
        using value_type = T;
		using reference = T&;
		using pointer = T*;
		using const_reference = const T&;
		using const_pointer = const T*;
		using size_type = size_t;

        lru_cache(size_t capacity) : _capacity(capacity) {}

        const_reference get(const key_type& key) const
        {            
            auto it = _map.at(key);
            _elevate_to_front(it);
            return *it;
        }

        const_pointer get_if(const key_type& key) const
        {
            if(!contains(key))
				return nullptr;
            
            auto it = _map.at(key);
            _elevate_to_front(it);
            return &*it;
        }

        void set(const key_type& key, const T& value)
        {
            // check if its an update action
            if(contains(key))
            {
                auto it = _map.at(key);
                *it = value; // update
                _elevate_to_front(it);
            }
            // or insert action
            else
            {
                _list.push_front(value);
                _map.insert(std::make_pair(key, _list.begin()));

                if(_map.size() > _capacity)
                    _evict_one();
            }
        }

        void resize(size_t new_capacity)
        {
            if(new_capacity < _capacity)
            {
                // evict extras
                _evict_n(_capacity - new_capacity);
            }
            _capacity = new_capacity;
        }
        

        auto contains(const key_type& key) const { return _map.contains(key); }
        auto count(const key_type& key) const { return _map.count(key); }
        auto empty(const key_type& key) const { return _map.empty(); }
        auto size() const { return _map.size(); }
        auto capacity() const { return _capacity; }

    private:
        map_type _map;
        mutable list_type _list;
        size_type _capacity;
    };
} // namespace vecl



#endif
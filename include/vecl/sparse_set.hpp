#ifndef VECL_SPARSE_SET_HPP
#define VECL_SPARSE_SET_HPP

#ifndef VECL_SPARSE_SIZE
#define VECL_SPARSE_SIZE 1024
#endif

#include "config/config.h"
#include "internal/vector.hpp"

#include <algorithm>
#include <tuple>

namespace vecl
{
    /**
     * @brief Sparse Set data structure. 
     * Provides constant time insert, remove, and lookup, while providing 
     * locality of reference.
     * 
     * @tparam Id_t Unsigned integer type.
     * @tparam Size Size of the underlying sparse array. Default value is 1024
     * and is provided by VECL_SPARSE_SIZE definition. Define VECL_SPARSE_SIZE 
     * to change the default value.
     */
    template<
        typename Id_t = uint32_t, 
        size_t Size = VECL_SPARSE_SIZE>
    class sparse_set
    {
    public:
        /** @brief Type traits  */
        using id_type = Id_t;
        using vector_type = vector<Id_t>;
        using key_type = Id_t;
        using value_type = Id_t;
        using size_type = size_t;
        using difference_type = typename vector_type::difference_type;
        using allocator_type = typename vector_type::allocator_type;
        using iterator = typename vector_type::const_iterator;
        using reverse_iterator = typename vector_type::const_reverse_iterator;

        static constexpr size_type size_value = Size;

        /**
         * @brief Default Constructor. 
         * 
         * @param mr Pointer to a pmr resource. Default gets the default
         * global pmr resource via get_default_resource().
         */
        sparse_set(
            std::pmr::memory_resource* mr = std::pmr::get_default_resource()) :
            _dense(mr), _sparse(Size, 0, mr) 
        {
            static_assert(std::is_unsigned_v<Id_t>, "Id_t must be unsigned!");
        }

        /**
         * @brief Range Constructor. 
         * 
         * @tparam It Iterator type. 
         * 
         * @param first Iterator to start of range.
         * @param last Iterator to end of range.
         * @param mr Pointer to a pmr resource. Default gets the default
         * global pmr resource via get_default_resource().
         * 
         * @todo TODO
         */
        /*template <typename It>
        sparse_set(
            It first, 
            It last, 
            std::pmr::memory_resource* mr = std::pmr::get_default_resource()) :
            sparse_set(mr) {}*/

        /**
         * @brief Initializer list Constructor. 
         * 
         * @param il Initializer list.
         * @param mr Pointer to a pmr resource. Default gets the default
         * global pmr resource via get_default_resource().
         * 
         * @todo TODO
         */
        /*sparse_set(
            std::initializer_list<value_type> il,
            std::pmr::memory_resource* mr = std::pmr::get_default_resource()) :
            sparse_set(mr) {}*/
        

        /**
         * @brief Standard Iterable Object boilerplate.
         * @return Iterator to beginning of range.
         */
        VECL_NODISCARD iterator begin() const { return _dense.cbegin(); }

        /**
         * @brief Standard Iterable Object boilerplate.
         * @return Iterator to end of range.
         */
        VECL_NODISCARD iterator end() const { return _dense.cend(); }

        /**
         * @brief Standard Iterable Object boilerplate.
         * @return Iterator to beginning of range.
         */
        VECL_NODISCARD iterator cbegin() const { return _dense.cbegin(); }

        /**
         * @brief Standard Iterable Object boilerplate.
         * @return Iterator to end of range.
         */
        VECL_NODISCARD iterator cend() const { return _dense.cend(); }

        /**
         * @brief Standard Iterable Object boilerplate.
         * @return Reverse Iterator to beginning of range.
         */
        VECL_NODISCARD iterator rbegin() const { return _dense.crbegin(); }

        /**
         * @brief Standard Iterable Object boilerplate.
         * @return Reverse Iterator to end of range.
         */
        VECL_NODISCARD iterator rend() const { return _dense.crend(); }

        /**
         * @brief Standard Iterable Object boilerplate.
         * @return Reverse Iterator to beginning of range.
         */
        VECL_NODISCARD iterator crbegin() const { return _dense.crbegin(); }
        
        /**
         * @brief Standard Iterable Object boilerplate.
         * @return Reverse Iterator to end of range.
         */
        VECL_NODISCARD iterator crend() const { return _dense.crend(); }

        /**
         * @return Size of underlying dense array.
         */
        VECL_NODISCARD size_type size() const { return _dense.size(); }

        /**
         * @brief Prefer static constexpr size_value.
         * 
         * @return Size of underlying sparse array. 
         */
        VECL_NODISCARD size_type max_size() const { return _sparse.size(); }

        /**
         * @return Size of underlying dense array.
         */
        VECL_NODISCARD size_type dense_size() const { return _dense.size(); }

        /**
         * @brief Prefer static constexpr size_value.
         * 
         * @return Size of underlying sparse array. 
         */
        VECL_NODISCARD size_type sparse_size() const { return _sparse.size(); }

        /**
         * @brief Checks if the container is empty.
         */
        VECL_NODISCARD bool empty() const { return _dense.empty(); }

        /**
         * @brief Accesses specified element at index by value. No bounds
         * check is performed.
         */
        VECL_NODISCARD value_type operator[](size_t index) const 
        { 
            return _dense[index]; 
        }

        /**
         * @brief Accesses specified element at index by value with bounds
         * checking.
         * @throw std::out_of_range if index is not within the range of the
         * container.
         */
        VECL_NODISCARD value_type at(size_t index) const 
        { 
            return _dense.at(index); 
        }

        /**
         * @brief Accesses specified element at index by const pointer with 
         * bounds checking. Returns nullptr if index is out of range.
         */
        VECL_NODISCARD const value_type* at_if(size_t index) const 
        { 
            if(index < size())
                return &_dense.at(index);
            return nullptr;
        }

        /**
         * @brief Accesses first element by value. Calling front on empty
         * container is undefined.
         */
        VECL_NODISCARD value_type front() const { return _dense.front(); }
        /**
         * @brief Accesses last element by value. Calling back on empty
         * container is undefined.
         */
        VECL_NODISCARD value_type back() const { return _dense.back(); }

        /**
         * @brief Direct access to underlying dense array by const pointer.
         */
        VECL_NODISCARD const value_type* data() const { return _dense.data(); }

        /**
         * @brief Sparse Set data structure specifically for Entity type. Provides
         * Contant time insert, remove, and lookup, while providing locality of
         * reference.
         */
        iterator emplace_back(key_type key)
        {
            if (!count(key))
            {
                _sparse[key] = static_cast<key_type>(_dense.size());
                _dense.emplace_back(key);
                return --end();
            }
            return begin() + _sparse[key];
        }

        /**
         * @brief Appends the key to the end of the container.
         */
        void push_back(key_type key) 
        {
            if (!count(key))
            {
                _sparse[key] = static_cast<key_type>(_dense.size());
                _dense.emplace_back(key);
            }
        }

        /**
         * @brief Removes the last key in the container. Calling pop_back on
         * empty container is undefined.
         */
        void pop_back() 
        {
            _sparse[_dense.back()] = 0;
            _dense.pop_back();
        }

        /**
         * @brief Inserts key into the container and returns an (iterator,
         * outcome) pair. 
         * Outcome is true if the key is successfully inserted nto the 
         * container; false if it already exists. 
         * Iterator is always valid and will either point to the newly inserted
         * element, or an existing element.
         * 
         * @return (Iterator, Outcome) pair.
         */
        std::pair<iterator, bool> insert(key_type key) 
        {
            if (!count(key))
            {
                _sparse[key] = static_cast<key_type>(_dense.size());
                _dense.emplace_back(key);
                return std::make_pair(--end(), true);
            }
            return std::make_pair(begin() + _sparse[key], false);
        }

        /**
         * @todo initializer list insert
         */
        /*iterator insert(std::initializer_list<value_type> il) {}*/

        /**
         * @brief Erases a key from the container and returns an iterator
         * to the replacing element. end() is returned if range is
         * empty after the operation or if the key is not found in the
         * container.
         * @return Iterator to replacing element.
         */
        iterator erase(key_type key) 
        {
            if (count(key))
            {
                auto other = _dense.back();
                _in_swap(key, other);
                _sparse[key] = 0;
                _dense.pop_back();
                return begin() + _sparse[other];
            }
            return end();
        }

        /**
         * @brief Erases an iterator from the container and returns an iterator
         * to the replacing element. end() is returned if range is
         * empty after the operation or if the key is not found in the
         * container.
         * @return Iterator to replacing element.
         */
        iterator erase(iterator position) 
        {
            auto from = *position;
            auto to = _dense.back();
            _in_swap(from, to);
            _sparse[from] = 0;
            _dense.pop_back();
            
            if (empty())
                return end();
            return position;
        }

        /**
         * @brief Removes a key from the iterator and returns the outcome
         * of the operation.
         * @return True if key was removed from the container.
         */
        bool remove(key_type key)
        {
            if (count(key))
            {
                auto other = _dense.back();
                _in_swap(key, other);
                _sparse[key] = 0;
                _dense.pop_back();
                return true;
            }
            return false;
        }

        /**
         * @brief Swaps the contents of two sparse sets.
         */
        void swap(sparse_set& x) 
        {
            if(&x != this)
            {
                std::swap(_dense, x._dense);
                std::swap(_sparse, x._sparse);
            }
        }

        /**
         * @brief Clears the sparse_set.
         */
        void clear() VECL_NOEXCEPT 
        {
            _dense.clear(); 
        }


        /**
         * @brief Searches for the key in the container and returns an iterator
         * to the found key. end() is returned if the key is not found.
         */
        iterator find(key_type key) const 
        { 
            if (count(key))
                return begin() + _sparse[key];
            return end();
        }

        /**
         * @brief Counts the number of times the key appears in the container.
         * @return 1 if key exists in the container; 0 if it doesn't.
         */
        size_type count(key_type key) const 
        {
            if (empty())
                return false;
            return _dense[_sparse[key]] == key; 
        }

        /**
         * @todo
         */
        //void sort();

    /**
     * @return Copy of allocator_type object used by the container.
     */
        VECL_NODISCARD allocator_type get_allocator() const VECL_NOEXCEPT 
        { 
            return _dense.get_allocator(); 
        }

    private:

        void _in_swap(Id_t rhs, Id_t lhs)
        {
            auto from = _sparse[lhs];
            auto to = _sparse[rhs];

            std::swap(_sparse[lhs], _sparse[rhs]);
            std::swap(_dense[from], _dense[to]);
        }

        vector<Id_t> _dense;
        vector<Id_t> _sparse;
    };
}

#endif
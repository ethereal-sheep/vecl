#ifndef VECL_SET_VECTOR_H
#define VECL_SET_VECTOR_H

#include "config/config.h"

#include <memory_resource> // pmr::memory_resource
#include <vector> // vector
#include <unordered_set> // unordered_set

namespace vecl
{

	/**
	 * @brief A Set Vector is a container adaptor that provide insertion-order
     * iteration, whilst maintaining constant time lookup.
	 *
	 * @tparam  T key type
     *          V vector type
     *          S set type
	 */
    template<
        typename T, 
        typename V = std::pmr::vector<T>, 
        typename S = std::pmr::unordered_set<T>
    >
    class set_vector
 	{
    public:
        /**
         * @note TYPE TRAITS
         */
		using vector_type = V;
		using set_type = S;
		using key_type = T;
		using const_reference = const T&;
		using const_pointer = const T*;
		using size_type = size_t;
		using difference_type = typename vector_type::difference_type;
		using allocator_type = std::pmr::polymorphic_allocator<std::byte>;
		using iterator = typename vector_type::const_iterator;
		using reverse_iterator = typename vector_type::const_reverse_iterator;

		/**
		 * @note MEMBER FUNCTIONS
		 */
        /**
         * @brief Default Constructor.
         *
         * @param mr Pointer to a pmr resource. Default gets the default
         * global pmr resource via get_default_resource().
         */
		explicit set_vector(
			allocator_type mr = std::pmr::get_default_resource()
		) :
			_vec(mr), _set(mr)
		{
		}

        /**
		 * @brief Range Constructor.
		 *
		 * @tparam It Iterator type.
		 *
		 * @param first Iterator to start of range.
		 * @param last Iterator to end of range.
		 * @param capacity Capacity of the map.
		 * @param mr Pointer to a pmr resource. Default gets the default
		 * global pmr resource via get_default_resource().
		 */
		template <typename It>
		set_vector(
			It first, It last,
			allocator_type mr = std::pmr::get_default_resource()
		) :
			_vec(mr), _set(mr)
		{
			for (; first != last; ++first)
				push_back(static_cast<key_type>(*first));
		}


        /**
		 * @brief Initializer list Constructor.
		 *
		 * @param il Initializer list.
		 * @param capacity Capacity of the map.
		 * @param mr Pointer to a pmr resource. Default gets the default
		 * global pmr resource via get_default_resource().
		 */
		set_vector(
			std::initializer_list<T> il,
			allocator_type mr = std::pmr::get_default_resource()
		) :
			set_vector(il.begin(), il.end(), mr)
		{
		}

		/**
		 * @brief Default Copy Constructor. Uses same memory resource as
		 * other.
		 */
		set_vector(const set_vector& other) = default;

		/**
		 * @brief Memory-Extended Copy Constructor. Uses provided
		 * memory_resource to allocate copied arrays from other.
		 *
		 * @param other Const-reference to other.
		 * @param mr Pointer to a pmr resource.
		 */
		set_vector(
			const set_vector & other,
			allocator_type mr
		) :
			_vec(other._vec, mr), _set(other._set, mr)
		{
		}

		/**
		 * @brief Default Move Constructor. Constructs container with
		 * the contents of other using move-semantics. After the move, other
		 * is guaranteed to be empty.
		 */
		set_vector(set_vector && other) = default;

		/**
		 * @brief Memory-Extended Move Constructor. If memory_resource used
		 * by other is not the same as memory_resource provided, the
		 * construction resolves to a Memory-Extended copy construction. In
		 * which case, other is not guranteed to be empty after the move.
		 *
		 * @param other Universal-reference to other.
		 * @param mr Pointer to a pmr resource.
		 */
		set_vector(
			set_vector && other,
			allocator_type mr
		) :
			_vec(std::move(other._vec), mr),
			_set(std::move(other._set), mr)
		{
		}

		/**
		 * @brief Copy-Assignment Operator. Uses same memory resource as
		 * other. If the memory_resource of this is equal to that of other,
		 * the memory owned by this may be reused when possible.
		 *
		 * @param other Const-reference to other.
		 */
		set_vector& operator=(const set_vector & other) = default;

		/**
		 * @brief Move-Assignment Operator. Uses same memory resource as
		 * other. If the memory_resource of this is equal to that of other,
		 * the memory owned by this may be reused when possible. Else, the
		 * assignment resolves to a copy-assignment.
		 *
		 * @param other Universal-reference to other.
		 */
		set_vector& operator=(set_vector && other) = default;

		/**
		 * @brief Replaces the contents with those identified by il.
		 *
		 * @tparam T Value type of initializer list.
		 *
		 * @param il Initializer list.
		 */
		set_vector& operator=(std::initializer_list<T> il)
		{
			clear();
			for (auto i : il)
				push_back(i);
		}

		/**
		 * @brief Swaps the contents of two sparse sets. The swap operation
		 * of two set_vectors with different memory_resource is undefined.
		 */
		void swap(set_vector& rhs)
		{
			if (&rhs != this)
			{
				std::swap(_vec, rhs._vec);
				std::swap(_set, rhs._set);
			}
		}

		/**
		 * @return Copy of allocator_type object used by the container.
		 */
		VECL_NODISCARD allocator_type get_allocator() const VECL_NOEXCEPT
		{
			return _vec.get_allocator();
		}

		/**
		 * @note ITERATORS
		 */

        /**
         * @brief Standard Iterable Object boilerplate.
         * @return Iterator to beginning of range.
         */
		VECL_NODISCARD iterator begin() const VECL_NOEXCEPT
		{
			return _vec.cbegin();
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Iterator to end of range.
		 */
		VECL_NODISCARD iterator end() const VECL_NOEXCEPT
		{
			return _vec.cend();
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Iterator to beginning of range.
		 */
		VECL_NODISCARD iterator cbegin() const VECL_NOEXCEPT
		{
			return _vec.cbegin();
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Iterator to end of range.
		 */
		VECL_NODISCARD iterator cend() const VECL_NOEXCEPT
		{
			return _vec.cend();
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Reverse Iterator to beginning of range.
		 */
		VECL_NODISCARD iterator rbegin() const VECL_NOEXCEPT
		{
			return _vec.crbegin();
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Reverse Iterator to end of range.
		 */
		VECL_NODISCARD iterator rend() const VECL_NOEXCEPT
		{
			return _vec.crend();
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Reverse Iterator to beginning of range.
		 */
		VECL_NODISCARD iterator crbegin() const VECL_NOEXCEPT
		{
			return _vec.crbegin();
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Reverse Iterator to end of range.
		 */
		VECL_NODISCARD iterator crend() const VECL_NOEXCEPT
		{
			return _vec.crend();
		}


		/**
		 * @note CAPACITY
		 */

		 /**
		  * @return Size of underlying dense array.
		  */
		VECL_NODISCARD size_type size() const VECL_NOEXCEPT
		{
			return _vec.size();
		}

		/**
		 * @brief Checks if the container is empty.
		 */
		VECL_NODISCARD bool empty() const VECL_NOEXCEPT { return _vec.empty(); }


		/**
		 * @note ELEMENT ACCESS
		 */

		/**
		 * @brief Subscript operator.
		*/
		VECL_NODISCARD const_reference operator[](size_type i) const
		{
			VECL_ASSERT(i < size());
			return begin()[i];
		}
		/**
		 * @brief Accesses first element by value. Calling front on empty
		 * container is undefined.
		 */
		VECL_NODISCARD const_reference front() const 
        { 
			VECL_ASSERT(!empty());
            return _vec.front(); 
        }
		/**
		 * @brief Accesses last element by value. Calling back on empty
		 * container is undefined.
		 */
		VECL_NODISCARD const_reference back() const 
        { 
			VECL_ASSERT(!empty());
            return _vec.back(); 
        }

		/**
		 * @brief Direct access to underlying dense array by const pointer.
		 */
		VECL_NODISCARD const_pointer data() const VECL_NOEXCEPT
		{
			return _vec.data();
		}


		/**
		 * @note MODIFIERS
		 */

        /**
         * @brief Clears the set_vector.
         */
		void clear() VECL_NOEXCEPT
		{
			_set.clear();
			_vec.clear();
		}



		/**
		 * @brief Construcs key with args and appends it to the end of the 
         * container.
		 *
		 * @return True if key was inserted.
		 */
		template<typename... Args>
		auto emplace_back(Args... args)
		{
			return insert(key_type(std::forward<Args>(args)...));
		}

		/**
		 * @brief Constructs key with args and inserts it to the end of the 
         * container.
		 * 
		 * @return True if key was inserted.
         * 
         * @todo change to emplacing at iterator?
		 */
		template<typename... Args>
		auto emplace(Args... args)
		{
			return insert(key_type(std::forward<Args>(args)...));
		}

		/**
		 * @brief Appends the key to the end of the container.
		 */
		void push_back(const key_type& key)
		{
            insert(key);
		}

		/**
		 * @brief Removes the last key in the container. Calling pop_back on
		 * empty container is undefined.
		 */
		void pop_back()
		{
			VECL_ASSERT(!empty());
            _set.erase(back());
            _vec.pop_back();
		}

		/**
		 * @brief Inserts key into the container.
		 *
		 * @return True if key was inserted.
		 */
		bool insert(key_type val)
		{
            bool result = _set.insert(val).second;
            if (result)
                _vec.push_back(val);
            return result;
		}
		/**
		 * @brief Inserts range of keys into the container.
		 */
        template<typename It>
        void insert(It s, It e) 
        {
            for (; s != e; ++s)
                if (_set.insert(*s).second)
                    _vec.push_back(*s);
        }
		/**
		 * @brief Inserts initializer list into the container.
		 */
        void insert(std::initializer_list<key_type> il) 
        {
            insert(il.begin(), il.end());
        }

		/**
		 * @brief Erases a key from the container and returns an iterator
		 * to the replacing element.
		 * If the key is not found in the container; or
		 * if the container is empty after the operation,
		 * end() is returned.
		 *
		 * @return Iterator to replacing element.
		 */
		iterator erase(key_type key)
		{
			if (count(key))
			{
                auto it = find(key);
                VECL_ASSERT(it != end());

                _set.erase(key);
                return _vec.erase(it);
			}
			return end();
		}

		/**
		 * @brief Erases an iterator from the container and returns an iterator
		 * to the replacing element. If container is empty after the operation,
		 * end() is returned.
		 * @return Iterator to replacing element.
		 */
		iterator erase(iterator position)
		{
            auto& key = *position;
            VECL_ASSERT(_set.count(key));
            _set.erase(key);

            return _vec.erase(position);
		}

		/**
		 * @brief Removes a key from the container and returns the outcome
		 * of the operation.
		 * @return True if key was removed from the container.
		 */
		bool remove(key_type key)
		{
			if (_set.erase(key).second())
			{
                auto it = find(key);
                VECL_ASSERT(it != end());
                _vec.erase(it);

                return true;
			}
			return false;
		}

        
		/**
		 * @note LOOKUP
		 */

		 /**
		  * @brief Searches for the key in the container and returns an iterator
		  * to the found key. end() is returned if the key is not found.
		  */
		iterator find(const key_type& key) const
		{
			if (count(key))
				return std::find(begin(), end(), key);
			return end();
		}

		/**
		 * @brief Counts the number of times the key appears in the container.
		 * @return 1 if key exists in the container; 0 if it doesn't.
		 */
		size_type count(key_type key) const
		{
			return _set.count(key);
		}

		/**
		 * @brief Checks if key is in the container.
		 */
		bool contains(key_type key) const
		{
			return _set.contains(key);
		}


        
		/**
		 * @note NON-MEMBER FUNCTIONS
		 */

        /**
         * @brief Operator== overload. Compares keys orderwise within the
         * container. See equal() for set equality comparison function.
         */
		friend bool operator==(
			const set_vector & lhs,
			const set_vector & rhs)
		{
			return lhs._vec == rhs._vec;
		}
		/**
		 * @brief Operator!= overload. Compares keys orderwise within the
		 * container.
		 */
		friend bool operator!=(
			const set_vector & lhs,
			const set_vector & rhs)
		{
			return lhs._vec != rhs._vec;
		}

		/**
		 * @brief Swaps the contents of two sparse sets. The swap operation
		 * of two sets with different memory_resource is undefined.
		 */
		friend void swap(set_vector & lhs, set_vector & rhs) VECL_NOEXCEPT
		{
			lhs.swap(rhs);
		}

        private:
            vector_type _vec;
            set_type _set;


    };
}

#endif
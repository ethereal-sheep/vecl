#ifndef VECL_SET_VECTOR_H
#define VECL_SET_VECTOR_H

#include "config/config.h"

#include <vector>
#include <unordered_set>
#include <memory>

// namespace vecl
// {
// 	template<typename T>
// 	class set_vector
// 	{
// 	public:
// 		/**
// 		 * @note TYPE TRAITS
// 		 */
// 		using vector_t = std::pmr::vector<T>;
// 		using set_t = std::pmr::unordered_set<T>;
// 		using key_type = T;
// 		using value_type = T;
// 		using size_type = size_t;
// 		using difference_type = typename vector_t::difference_type;
// 		using allocator_type = std::pmr::polymorphic_allocator<std::byte>;
// 		using iterator = typename vector_t::const_iterator;
// 		using reverse_iterator = typename vector_t::const_reverse_iterator;

// 		/**
// 		 * @note MEMBER FUNCTIONS
// 		 */

// 		 /**
// 		  * @brief Default Constructor.
// 		  *
// 		  * @param mr Pointer to a pmr resource. Default gets the default
// 		  * global pmr resource via get_default_resource().
// 		  */
// 		explicit set_vector(
// 			allocator_type mr = std::pmr::get_default_resource()
// 		) :
// 			_vec(mr), _set(mr)
// 		{
// 		}

// 		/**
// 		 * @brief Range Constructor.
// 		 *
// 		 * @tparam It Iterator type.
// 		 *
// 		 * @param first Iterator to start of range.
// 		 * @param last Iterator to end of range.
// 		 * @param capacity Capacity of the map.
// 		 * @param mr Pointer to a pmr resource. Default gets the default
// 		 * global pmr resource via get_default_resource().
// 		 */
// 		template <typename It>
// 		set_vector(
// 			It first, It last,
// 			allocator_type mr = std::pmr::get_default_resource()
// 		) :
// 			_vec(mr), _set(mr)
// 		{
// 			for (; first != last; ++first)
// 				push_back(static_cast<key_type>(*first));
// 		}

// 		/**
// 		 * @brief Initializer list Constructor.
// 		 *
// 		 * @tparam T Value type of initalizer list.
// 		 *
// 		 * @param il Initializer list.
// 		 * @param capacity Capacity of the map.
// 		 * @param mr Pointer to a pmr resource. Default gets the default
// 		 * global pmr resource via get_default_resource().
// 		 */
// 		template <typename T>
// 		set_vector(
// 			std::initializer_list<T> il,
// 			allocator_type mr = std::pmr::get_default_resource()
// 		) :
// 			set_vector(il.begin(), il.end(), mr)
// 		{
// 		}

// 		/**
// 		 * @brief Default Copy Constructor. Uses same memory resource as
// 		 * other.
// 		 */
// 		set_vector(const set_vector& other) = default;

// 		/**
// 		 * @brief Memory-Extended Copy Constructor. Uses provided
// 		 * memory_resource to allocate copied arrays from other.
// 		 *
// 		 * @param other Const-reference to other.
// 		 * @param mr Pointer to a pmr resource.
// 		 */
// 		set_vector(
// 			const set_vector & other,
// 			allocator_type mr
// 		) :
// 			_vec(other._vec, mr), _set(other._set, mr)
// 		{
// 		}

// 		/**
// 		 * @brief Default Move Constructor. Constructs container with
// 		 * the contents of other using move-semantics. After the move, other
// 		 * is guaranteed to be empty.
// 		 */
// 		set_vector(set_vector && other) = default;

// 		/**
// 		 * @brief Memory-Extended Move Constructor. If memory_resource used
// 		 * by other is not the same as memory_resource provided, the
// 		 * construction resolves to a Memory-Extended copy construction. In
// 		 * which case, other is not guranteed to be empty after the move.
// 		 *
// 		 * @param other Universal-reference to other.
// 		 * @param mr Pointer to a pmr resource.
// 		 */
// 		set_vector(
// 			set_vector && other,
// 			allocator_type mr
// 		) :
// 			_vec(std::move(other._vec), mr),
// 			_set(std::move(other._set), mr)
// 		{

// 		}

// 		/**
// 		 * @brief Copy-Assignment Operator. Uses same memory resource as
// 		 * other. If the memory_resource of this is equal to that of other,
// 		 * the memory owned by this may be reused when possible.
// 		 *
// 		 * @param other Const-reference to other.
// 		 */
// 		set_vector& operator=(const set_vector & other) = default;

// 		/**
// 		 * @brief Move-Assignment Operator. Uses same memory resource as
// 		 * other. If the memory_resource of this is equal to that of other,
// 		 * the memory owned by this may be reused when possible. Else, the
// 		 * assignment resolves to a copy-assignment.
// 		 *
// 		 * @param other Universal-reference to other.
// 		 */
// 		set_vector& operator=(set_vector && other) = default;

// 		/**
// 		 * @brief Replaces the contents with those identified by il.
// 		 *
// 		 * @tparam T Value type of initializer list.
// 		 *
// 		 * @param il Initializer list.
// 		 */
// 		template<typename T>
// 		set_vector& operator=(std::initializer_list<T> il)
// 		{
// 			clear();
// 			for (auto i : il)
// 				push_back(i);
// 		}

// 		/**
// 		 * @brief Swaps the contents of two sparse sets. The swap operation
// 		 * of two set_vectors with different memory_resource is undefined.
// 		 */
// 		void swap(set_vector & x)
// 		{
// 			if (&x != this)
// 			{
// 				std::swap(_vec, x._vec);
// 				std::swap(_set, x._set);
// 			}
// 		}

// 		/**
// 		 * @return Copy of allocator_type object used by the container.
// 		 */
// 		VECL_NODISCARD allocator_type get_allocator() const VECL_NOEXCEPT
// 		{
// 			return _vec.get_allocator();
// 		}

// 		/**
// 		 * @note ITERATORS
// 		 */

// 		 /**
// 		  * @brief Standard Iterable Object boilerplate.
// 		  * @return Iterator to beginning of range.
// 		  */
// 		VECL_NODISCARD iterator begin() const VECL_NOEXCEPT
// 		{
// 			return _vec.cbegin();
// 		}

// 		/**
// 		 * @brief Standard Iterable Object boilerplate.
// 		 * @return Iterator to end of range.
// 		 */
// 		VECL_NODISCARD iterator end() const VECL_NOEXCEPT
// 		{
// 			return _vec.cend();
// 		}

// 		/**
// 		 * @brief Standard Iterable Object boilerplate.
// 		 * @return Iterator to beginning of range.
// 		 */
// 		VECL_NODISCARD iterator cbegin() const VECL_NOEXCEPT
// 		{
// 			return _vec.cbegin();
// 		}

// 		/**
// 		 * @brief Standard Iterable Object boilerplate.
// 		 * @return Iterator to end of range.
// 		 */
// 		VECL_NODISCARD iterator cend() const VECL_NOEXCEPT
// 		{
// 			return _vec.cend();
// 		}

// 		/**
// 		 * @brief Standard Iterable Object boilerplate.
// 		 * @return Reverse Iterator to beginning of range.
// 		 */
// 		VECL_NODISCARD iterator rbegin() const VECL_NOEXCEPT
// 		{
// 			return _vec.crbegin();
// 		}

// 		/**
// 		 * @brief Standard Iterable Object boilerplate.
// 		 * @return Reverse Iterator to end of range.
// 		 */
// 		VECL_NODISCARD iterator rend() const VECL_NOEXCEPT
// 		{
// 			return _vec.crend();
// 		}

// 		/**
// 		 * @brief Standard Iterable Object boilerplate.
// 		 * @return Reverse Iterator to beginning of range.
// 		 */
// 		VECL_NODISCARD iterator crbegin() const VECL_NOEXCEPT
// 		{
// 			return _vec.crbegin();
// 		}

// 		/**
// 		 * @brief Standard Iterable Object boilerplate.
// 		 * @return Reverse Iterator to end of range.
// 		 */
// 		VECL_NODISCARD iterator crend() const VECL_NOEXCEPT
// 		{
// 			return _vec.crend();
// 		}


// 		/**
// 		 * @note CAPACITY
// 		 */

// 		 /**
// 		  * @return Size of underlying dense array.
// 		  */
// 		VECL_NODISCARD size_type size() const VECL_NOEXCEPT
// 		{
// 			return _vec.size();
// 		}

// 		/**
// 		 * @brief Checks if the container is empty.
// 		 */
// 		VECL_NODISCARD bool empty() const VECL_NOEXCEPT { return _vec.empty(); }


// 		/**
// 		 * @note ELEMENT ACCESS
// 		 */

// 		/**
// 		 * @brief Accesses first element by value. Calling front on empty
// 		 * container is undefined.
// 		 */
// 		VECL_NODISCARD value_type front() const { return _vec.front(); }
// 		/**
// 		 * @brief Accesses last element by value. Calling back on empty
// 		 * container is undefined.
// 		 */
// 		VECL_NODISCARD value_type back() const { return _vec.back(); }

// 		/**
// 		 * @brief Direct access to underlying dense array by const pointer.
// 		 */
// 		VECL_NODISCARD const value_type* data() const VECL_NOEXCEPT
// 		{
// 			return _vec.data();
// 		}

// 		/**
// 		 * @note MODIFIERS
// 		 */

// 		 /**
// 		  * @brief Clears the set_vector.
// 		  */
// 		void clear() VECL_NOEXCEPT
// 		{
// 			_set.clear();
// 			_vec.clear();
// 		}

// 		/**
// 		 * @brief Appends the key to the end of the container and returns an
// 		 * (iterator, outcome) pair.
// 		 *
// 		 * (1) Key is emplaced successfully;
// 		 * outcome is true and returning iterator points to newly created key.
// 		 * (2) Key is contained in container;
// 		 * outcome is false and returning iterator points to existing key.
// 		 * (3) Key value > max();
// 		 * outcome is false and returning iterator == end().
// 		 *
// 		 * @return (Iterator, Outcome) pair.
// 		 */
// 		template<typename... Args>
// 		std::pair<iterator, bool> emplace_back(key_type key)
// 		{
// 			return insert(key);
// 		}

// 		/**
// 		 * @brief Appends the key to the end of the container. If key is
// 		 * invalid or exists in the container, nothing happens.
// 		 */
// 		void push_back(key_type key)
// 		{
// 		}

// 		/**
// 		 * @brief Removes the last key in the container. Calling pop_back on
// 		 * empty container is undefined.
// 		 */
// 		void pop_back()
// 		{
// 		}

// 		/**
// 		 * @brief Inserts key into the container and returns an (iterator,
// 		 * outcome) pair.
// 		 *
// 		 * (1) Key is inserted successfully;
// 		 * outcome is true and returning iterator points to newly created key.
// 		 * (2) Key is contained in container;
// 		 * outcome is false and returning iterator points to existing key.
// 		 * (3) Key value > max();
// 		 * outcome is false and returning iterator == end().
// 		 *
// 		 * @return (Iterator, Outcome) pair.
// 		 */
// 		std::pair<iterator, bool> insert(value_type val)
// 		{
// 			/*if (!valid(key))
// 				return std::make_pair(end(), false);

// 			if (!count(key))
// 			{
// 				_dense.emplace_back(key);
// 				_sparse[key] = static_cast<key_type>(_dense.size() - 1);
// 				return std::make_pair(--end(), true);
// 			}*/
// 			return std::make_pair(begin(), false);
// 		}

// 		/**
// 		 * @todo initializer list insert
// 		 */
// 		 /*iterator insert(std::initializer_list<value_type> il) {}*/

// 		/**
// 		 * @brief Erases a key from the container and returns an iterator
// 		 * to the replacing element.
// 		 * If the key is either
// 		 * invalid or not found in the container; or
// 		 * if the container is empty after the operation,
// 		 * end() is returned.
// 		 *
// 		 * @return Iterator to replacing element.
// 		 */
// 		iterator erase(value_type key)
// 		{
// 			/*if (count(key))
// 			{
// 				auto other = _dense.back();
// 				_swap(key, other);
// 				_sparse[key] = 0;
// 				_dense.pop_back();
// 				return begin() + _sparse[other];
// 			}*/
// 			return end();
// 		}

// 		/**
// 		 * @brief Erases an iterator from the container and returns an iterator
// 		 * to the replacing element. If container is empty after the operation,
// 		 * end() is returned.
// 		 * @return Iterator to replacing element.
// 		 */
// 		iterator erase(iterator position)
// 		{
// 			/*auto from = *position;
// 			auto to = _dense.back();
// 			_swap(from, to);
// 			_sparse[from] = 0;
// 			_dense.pop_back();

// 			if (empty())
// 				return end();*/
// 			return position;
// 		}

// 		/**
// 		 * @brief Removes a key from the container and returns the outcome
// 		 * of the operation.
// 		 * @return True if key was removed from the container.
// 		 */
// 		bool remove(key_type key)
// 		{
// 			/*if (count(key))
// 			{
// 				auto other = _dense.back();
// 				_swap(key, other);
// 				_sparse[key] = 0;
// 				_dense.pop_back();
// 				return true;
// 			}*/
// 			return false;
// 		}


// 		/**
// 		 * @brief Sorts the underlying dense array in ascending order. Calls
// 		 * sort(std::less).
// 		 */
// 		void sort()
// 		{
// 			sort(std::less<T>{});
// 		}

// 		/**
// 		 * @brief Sorts the underlying dense array given a predicate function.
// 		 *
// 		 * @tparam Pred Predicate function type.
// 		 * @param pred Predicate function.
// 		 *
// 		 * @todo Optimize by using sparse as the copy array, reducing space
// 		 * to O(1).
// 		 */
// 		template <typename Pred>
// 		void sort(Pred && pred)
// 		{
// 			/*std::vector<Id> copy(size());
// 			std::iota(copy.begin(), copy.end(), 0);
// 			std::sort(copy.begin(), copy.end(),
// 				[&pred, this](const auto l, const auto r)
// 				{
// 					return pred(_dense[l], _dense[r]);
// 				});

// 			for (Id i = 0, len = static_cast<Id>(size()); i < len; ++i)
// 			{
// 				auto curr = i;
// 				auto next = copy[curr];

// 				while (curr != next)
// 				{
// 					_swap(_dense[copy[curr]], _dense[copy[next]]);
// 					copy[curr] = curr;
// 					curr = next;
// 					next = copy[curr];
// 				}
// 			}*/
// 		}

// 		/**
// 		 * @note LOOKUP
// 		 */

// 		 /**
// 		  * @brief Searches for the key in the container and returns an iterator
// 		  * to the found key. end() is returned if the key is not found.
// 		  */
// 		iterator find(key_type key) const
// 		{
// 			/*if (count(key))
// 				return begin() + _sparse[key];*/
// 			return end();
// 		}

// 		/**
// 		 * @brief Counts the number of times the key appears in the container.
// 		 * @return 1 if key exists in the container; 0 if it doesn't.
// 		 */
// 		size_type count(value_type key) const
// 		{
// 			return _set.count(key);
// 		}

// 		/**
// 		 * @brief Checks if key is in the container.
// 		 */
// 		bool contains(key_type key) const
// 		{
// 			return _set.contains(key);
// 		}


// 		/**
// 		 * @note SET OPERATIONS
// 		 */

// 		 /**
// 		  * @brief Set Equal operation.
// 		  * @returns True if all keys in this is contained in other.
// 		  */
// 		template<typename TOther>
// 		bool set_equal(const set_vector<TOther>&other) const
// 		{
// 			if (size() != other.size())
// 				return false;

// 			for (auto id : other)
// 				if (!count(static_cast<key_type>(id)))
// 					return false;

// 			return true;
// 		}

// 		/**
// 		 * @brief Set Equal operation.
// 		 * @returns True if all keys in this is contained in other.
// 		 */
// 		template<typename It>
// 		bool set_equal(It start, It end) const
// 		{
// 			for (; start != end; ++start)
// 				if (!count(static_cast<key_type>(*start)))
// 					return false;

// 			return true;
// 		}

// 		/**
// 		 * @brief Merge operation. Merges other into the container.
// 		 */
// 		template<typename TOther>
// 		void merge(const set_vector<TOther>&other)
// 		{
// 			for (auto i : other)
// 				push_back(i);
// 		}

// 		/**
// 		 * @brief Intersect operation. Removes any key in container
// 		 * not contained in other.
// 		 */
// 		template<typename TOther>
// 		void intersect(const set_vector<TOther>&other)
// 		{
// 			auto it = begin();
// 			while (it != end())
// 			{
// 				if (!other.count(*it))
// 					it = erase(*it);
// 				else
// 					++it;
// 			}
// 		}

// 		/**
// 		 * @note NON-MEMBER FUNCTIONS
// 		 */

// 		 /**
// 		  * @brief Operator== overload. Compares keys orderwise within the
// 		  * container. See equal() for set equality comparison function.
// 		  */
// 		friend bool operator==(
// 			const set_vector & lhs,
// 			const set_vector & rhs)
// 		{
// 			return lhs._dense == rhs._dense;
// 		}
// 		/**
// 		 * @brief Operator!= overload. Compares keys orderwise within the
// 		 * container.
// 		 */
// 		friend bool operator!=(
// 			const set_vector & lhs,
// 			const set_vector & rhs)
// 		{
// 			return !(lhs == rhs);
// 		}

// 		/**
// 		 * @brief Swaps the contents of two sparse sets. The swap operation
// 		 * of two sets with different memory_resource is undefined.
// 		 */
// 		friend void swap(set_vector & lhs, set_vector & rhs) VECL_NOEXCEPT
// 		{
// 			lhs.swap(rhs);
// 		}


// 	private:
// 		vector_t _vec;
// 		set_t _set;

// 	};
// }
#endif
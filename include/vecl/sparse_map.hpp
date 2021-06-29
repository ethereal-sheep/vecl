#ifndef VECL_SPARSE_SET_HPP
#define VECL_SPARSE_SET_HPP

#ifndef VECL_SPARSE_SIZE
#define VECL_SPARSE_SIZE 1024
#endif

#include "config/config.h"

#include <memory_resource> // pmr::memory_resource
#include <vector> // pmr::vector
#include <algorithm> // swap
#include <tuple> // pair
#include <numeric> // iota

namespace vecl
{
	/**
	 * @brief Sparse Set data structure.
	 * Provides constant time insert, remove, and lookup, while providing
	 * locality of reference.
	 *
	 * @tparam Id_t Unsigned integer type.
	 */
	template <
		typename Id_t,
		typename T>
	class sparse_map
	{
		static_assert(
			std::is_unsigned_v<Id_t>,
			"Id_t must be an unsigned integral type!");

	public:
		/**
		 * @note TYPE TRAITS
		 */
		using id_type = Id_t;
		using key_type = Id_t;
		using mapped_type = T;
		using value_type = std::pair<const Id_t, T>;
		using size_type = size_t;
		using difference_type = ptrdiff_t;

		using dense_array = std::pmr::vector<value_type>;
		using sparse_array = std::pmr::vector<Id_t>;
		using allocator_type = typename dense_array::allocator_type;
		using iterator = typename dense_array::const_iterator;
		using reverse_iterator = typename dense_array::const_reverse_iterator;

		/**
		 * @note MEMBER FUNCTIONS
		 */

		 /**
		  * @brief Default Constructor. Default sparse array size is 1024
		  * and is provided by VECL_SPARSE_SIZE definition. Define
		  * VECL_SPARSE_SIZE to change the default value.
		  *
		  * @param mr Pointer to a pmr resource. Default gets the default
		  * global pmr resource via get_default_resource().
		  */
		explicit sparse_map(
			std::pmr::memory_resource* mr = std::pmr::get_default_resource()
		) :
			_dense(mr), _sparse(VECL_SPARSE_SIZE, 0, mr)
		{
		}
		/**
		 * @brief Explicit Size Constructor. Explicitly sets the size of the
		 * spares array and reserves the same amount in the dense array.
		 *
		 * @param size Size of the underlying sparse array.
		 * @param mr Pointer to a pmr resource. Default gets the default
		 * global pmr resource via get_default_resource().
		 */
		explicit sparse_map(
			size_type max_size,
			std::pmr::memory_resource* mr = std::pmr::get_default_resource()
		) :
			_dense(mr), _sparse(max_size, 0, mr)
		{
			_dense.reserve(max_size);
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
		 */
		template <typename It>
		sparse_map(
			It first, It last,
			size_type max_size = VECL_SPARSE_SIZE,
			std::pmr::memory_resource* mr = std::pmr::get_default_resource()
		) :
			_dense(mr), _sparse(max_size, 0, mr)
		{
			for (; first != last; ++first)
				push_back(static_cast<value_type>(*first));
		}

		/**
		 * @brief Initializer list Constructor.
		 *
		 * @tparam T Value type of initalizer list.
		 *
		 * @param il Initializer list.
		 * @param mr Pointer to a pmr resource. Default gets the default
		 * global pmr resource via get_default_resource().
		 */
		sparse_map(
			std::initializer_list<value_type> il,
			size_type max_size = VECL_SPARSE_SIZE,
			std::pmr::memory_resource* mr = std::pmr::get_default_resource()
		) :
			sparse_map(il.begin(), il.end(), max_size, mr)
		{
		}

		/**
		 * @brief Default Copy Constructor. Uses same memory resource as
		 * other.
		 *
		 * @param other Const-Ref to other.
		 */
		sparse_map(const sparse_map& other) = default;

		/**
		 * @brief Memory-Extended Copy Constructor. Uses provided
		 * memory_resource to allocate copied arrays from other.
		 *
		 * @param other Const-Ref to other.
		 * @param mr Pointer to a pmr resource.
		 */
		sparse_map(
			const sparse_map& other,
			std::pmr::memory_resource* mr
		) :
			_dense(other._dense, mr), _sparse(other._sparse, mr)
		{
		}

		/**
		 * @brief Default Move Constructor. Constructs container with
		 * the contents of other using move-semantics. After the move, other
		 * is guaranteed to be empty().
		 *
		 * @param other Universal-Ref to other.
		 */
		sparse_map(sparse_map&& other) = default;

		/**
		 * @brief Memory-Extended Move Constructor. If memory_resource used
		 * by other is not the same as memory_resource provided, the
		 * construction resolves to a Memory-Extended copy construction. In
		 * which case, other is not guranteed to be empty after the move.
		 *
		 * @param other Universal-Ref to other.
		 * @param mr Pointer to a pmr resource.
		 */
		sparse_map(
			sparse_map&& other,
			std::pmr::memory_resource* mr
		) :
			_dense(std::move(other._dense), mr),
			_sparse(std::move(other._sparse), mr)
		{

		}

		/**
		 * @brief Copy-Assignment Operator. Uses same memory resource as
		 * other. If the memory_resource of this is equal to that of other,
		 * the memory owned by this may be reused when possible.
		 *
		 * @param other Const-Ref to other.
		 */
		sparse_map& operator=(const sparse_map& other) = default;

		/**
		 * @brief Move-Assignment Operator. Uses same memory resource as
		 * other. If the memory_resource of this is equal to that of other,
		 * the memory owned by this may be reused when possible. Else, the
		 * assignment resolves to a copy-assignment.
		 *
		 * @param other Universal-Ref to other.
		 */
		sparse_map& operator=(sparse_map&& other) = default;

		/**
		 * @brief Replaces the contents with those identified by il.
		 *
		 * @tparam T Value type of initializer list.
		 *
		 * @param il Initializer list.
		 */
		sparse_map& operator=(std::initializer_list<value_type> il)
		{
			clear();
			for (auto it = il.begin(), end = il.end(); it != end; ++it)
				push_back(*it);
		}

		/**
		 * @brief Swaps the contents of two sparse sets. The swap operation
		 * of two sparse_sets with different memory_resource is undefined.
		 */
		void swap(sparse_map& x)
		{
			if (&x != this)
			{
				std::swap(_dense, x._dense);
				std::swap(_sparse, x._sparse);
			}
		}

		/**
		 * @return Copy of allocator_type object used by the container.
		 */
		VECL_NODISCARD allocator_type get_allocator() const VECL_NOEXCEPT
		{
			return _dense.get_allocator();
		}

		/**
		 * @note ITERATORS
		 */
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
		 * @note CAPACITY
		 */

		 /**
		  * @return Size of underlying dense array.
		  */
		VECL_NODISCARD size_type size() const VECL_NOEXCEPT
		{
			return _dense.size();
		}

		/**
		 * @return Size of underlying sparse array.
		 */
		VECL_NODISCARD size_type max_size() const VECL_NOEXCEPT
		{
			return _sparse.size();
		}

		/**
		 * @return Size of underlying dense array.
		 */
		VECL_NODISCARD size_type dense_size() const VECL_NOEXCEPT
		{
			return _dense.size();
		}

		/**
		 * @return Size of underlying sparse array.
		 */
		VECL_NODISCARD size_type sparse_size() const VECL_NOEXCEPT
		{
			return _sparse.size();
		}

		/**
		 * @return Greatest value a key can have in the container.
		 */
		VECL_NODISCARD key_type max() const VECL_NOEXCEPT
		{
			return static_cast<key_type>(_sparse.size() - 1);
		}

		/**
		 * @brief Checks if the key is a valid key in the container.
		 */
		VECL_NODISCARD bool valid(key_type key) const VECL_NOEXCEPT
		{
			return key <= max();
		}

		/**
		 * @brief Resizes the container. Resizing a non-empty container is
		 *  defined if and only if the maximum key value in the set is strictly
		 * less than new_size. Otherwise, the operation is undefined.
		 *
		 * @todo Decide if we should throw an exception when resizing to a
		 * smaller size.
		 */
		void resize(size_type new_size)
		{
			_sparse.resize(new_size, 0);
		}

		/**
		 * @brief Checks if the container is empty.
		 */
		VECL_NODISCARD bool empty() const VECL_NOEXCEPT { return _dense.empty(); }


		/**
		 * @note MODIFIERS
		 */

		 /**
		  * @brief Clears the sparse_set.
		  */
		void clear() VECL_NOEXCEPT
		{
			_dense.clear();
		}

		/**
		 * @brief Appends the key to the end of the container and returns an
		 * (iterator, outcome) pair.
		 *
		 * (1) Key is inserted successfully;
		 * outcome is true and returning iterator points to newly created key.
		 * (2) Key is contained in container;
		 * outcome is false and returning iterator points to existing key.
		 * (3) Key value > max();
		 * outcome is false and returning iterator == end().
		 *
		 * @return (Iterator, Outcome) pair.
		 */
		template <typename...Args>
		std::pair<iterator, bool> emplace_back(key_type key, Args&&... args)
		{
			if(count(key))

		}




		/**
		 * @note ELEMENT ACCESS
		 */

		 /**
		  * @brief Hashmap style subscript operator. Accesses specified
		  * element at index by reference if it exists. Else an insertion
			is performed. 
			(1)
		  */
		/*VECL_NODISCARD mapped_type& operator[](key_type key) const
		{
			return _dense[index];
		}*/

		/**
		 * @note LOOKUP
		 */

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
			if (empty() || !valid(key) || _sparse[key] >= size())
				return false;
			return _dense[_sparse[key]].first == key;
		}

		/**
		 * @brief Checks if key is in the container.
		 */
		bool contains(key_type key) const
		{
			return count(key);
		}

	private:
		void _swap(Id_t rhs, Id_t lhs)
		{
			auto from = _sparse[lhs];
			auto to = _sparse[rhs];

			std::swap(_sparse[lhs], _sparse[rhs]);
			std::swap(_dense[from], _dense[to]);
		}



		dense_array _dense;
		sparse_array _sparse;
	};
}

#endif
#ifndef VECL_SPARSE_SET_HPP
#define VECL_SPARSE_SET_HPP

#ifndef VECL_SPARSE_SIZE
#define VECL_SPARSE_SIZE 1024
#endif

#include "config/config.h"
#include "internal/vector.hpp"

#include <algorithm>
#include <tuple>
#include <numeric>

namespace vecl
{
	/**
	 * @brief Sparse Set data structure.
	 * Provides constant time insert, remove, and lookup, while providing
	 * locality of reference.
	 *
	 * @tparam Id_t Unsigned integer type.
	 */
	template<
		typename Id_t = uint32_t>
		class sparse_set
	{
		static_assert(std::is_unsigned_v<Id_t>, "Id_t must be unsigned!");

	public:
		/**
		 * @note TYPE TRAITS
		 */
		using id_type = Id_t;
		using vector_type = vector<Id_t>;
		using key_type = Id_t;
		using value_type = Id_t;
		using size_type = size_t;
		using difference_type = typename vector_type::difference_type;
		using allocator_type = typename vector_type::allocator_type;
		using iterator = typename vector_type::const_iterator;
		using reverse_iterator = typename vector_type::const_reverse_iterator;


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
		explicit sparse_set(
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
		explicit sparse_set(
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
		sparse_set(
			It first, It last,
			size_type max_size = VECL_SPARSE_SIZE,
			std::pmr::memory_resource* mr = std::pmr::get_default_resource()
		) :
			_dense(mr), _sparse(max_size, 0, mr)
		{
			for (; first != last; ++first)
				push_back(static_cast<key_type>(*first));
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
		template <typename T>
		sparse_set(
			std::initializer_list<T> il,
			size_type max_size = VECL_SPARSE_SIZE,
			std::pmr::memory_resource* mr = std::pmr::get_default_resource()
		) :
			_dense(mr), _sparse(max_size, 0, mr)
		{
			for (auto i : il)
				push_back(static_cast<key_type>(i));
		}

		/**
		 * @brief Default Copy Constructor. Uses same memory resource as
		 * other.
		 *
		 * @param other Const-Ref to other.
		 */
		sparse_set(const sparse_set& other) = default;

		/**
		 * @brief Memory-Extended Copy Constructor. Uses provided
		 * memory_resource to allocate copied arrays from other.
		 *
		 * @param other Const-Ref to other.
		 * @param mr Pointer to a pmr resource.
		 */
		sparse_set(
			const sparse_set& other,
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
		sparse_set(sparse_set&& other) = default;

		/**
		 * @brief Memory-Extended Move Constructor. If memory_resource used
		 * by other is not the same as memory_resource provided, the
		 * construction resolves to a Memory-Extended copy construction. In
		 * which case, other is not guranteed to be empty after the move.
		 *
		 * @param other Universal-Ref to other.
		 * @param mr Pointer to a pmr resource.
		 */
		sparse_set(
			sparse_set&& other,
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
		sparse_set& operator=(const sparse_set& other) = default;

		/**
		 * @brief Move-Assignment Operator. Uses same memory resource as
		 * other. If the memory_resource of this is equal to that of other,
		 * the memory owned by this may be reused when possible. Else, the
		 * assignment resolves to a copy-assignment.
		 *
		 * @param other Universal-Ref to other.
		 */
		sparse_set& operator=(sparse_set&& other) = default;

		/**
		 * @brief Replaces the contents with those identified by il.
		 *
		 * @tparam T Value type of initializer list.
		 *
		 * @param il Initializer list.
		 */
		template<typename T>
		sparse_set& operator=(std::initializer_list<T> il)
		{
			clear();
			for (auto i : il)
				push_back(i);
		}

		/**
		 * @brief Swaps the contents of two sparse sets. The swap operation
		 * of two sparse_sets with different memory_resource is undefined.
		 */
		void swap(sparse_set& x)
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
		VECL_NODISCARD size_type size() const { return _dense.size(); }

		/**
		 * @return Size of underlying sparse array.
		 */
		VECL_NODISCARD size_type max_size() const { return _sparse.size(); }

		/**
		 * @return Size of underlying dense array.
		 */
		VECL_NODISCARD size_type dense_size() const { return _dense.size(); }

		/**
		 * @return Size of underlying sparse array.
		 */
		VECL_NODISCARD size_type sparse_size() const { return _sparse.size(); }

		/**
		 * @return Greatest value a key can have in the container.
		 */
		VECL_NODISCARD key_type max() const
		{
			return static_cast<key_type>(_sparse.size() - 1);
		}

		/**
		 * @brief Checks if the key is a valid key in the container.
		 */
		VECL_NODISCARD bool valid(key_type key) const
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
		VECL_NODISCARD bool empty() const { return _dense.empty(); }


		/**
		 * @note ELEMENT ACCESS
		 */

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
		VECL_NODISCARD const value_type* at_if(size_t index) const VECL_NOEXCEPT
		{
			if (index < size())
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
		VECL_NODISCARD const value_type* data() const VECL_NOEXCEPT
		{
			return _dense.data();
		}

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
		 * @brief Appends the key to the end of the container and returns
		 * an iterator that points to the newly created key. If key is invalid
		 * or exists in the container, end() is returned.
		 */
		iterator emplace_back(key_type key)
		{
			if (!valid(key))
				return end();

			if (!count(key))
			{
				_sparse[key] = static_cast<key_type>(_dense.size());
				_dense.emplace_back(key);
				return --end();
			}
			return begin() + _sparse[key];
		}

		/**
		 * @brief Appends the key to the end of the container. If key is
		 * invalid or exists in the container, nothing happens.
		 */
		void push_back(key_type key)
		{
			if (!valid(key)) return;

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
		 * (1) Key is inserted successfully;
		 * outcome is true and returning iterator points to newly created key.
		 * (2) Key is contained in container;
		 * outcome is false and returning iterator points to existing key.
		 * (3) Key value > max();
		 * outcome is false and returning iterator == end().
		 *
		 * @return (Iterator, Outcome) pair.
		 */
		std::pair<iterator, bool> insert(key_type key)
		{
			if (!valid(key))
				return std::make_pair(end(), false);

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
		  * to the replacing element. end() is returned if the key is invalid,
		  * if the key is not found in the container, or if the container is
		  * empty after the operation.
		  *
		  * @return Iterator to replacing element.
		  */
		iterator erase(key_type key)
		{
			if (valid(key) && count(key))
			{
				auto other = _dense.back();
				_swap(key, other);
				_sparse[key] = 0;
				_dense.pop_back();
				return begin() + _sparse[other];
			}
			return end();
		}

		/**
		 * @brief Erases an iterator from the container and returns an iterator
		 * to the replacing element. end() is returned if container is
		 * empty after the operation.
		 * @return Iterator to replacing element.
		 */
		iterator erase(iterator position)
		{
			auto from = *position;
			auto to = _dense.back();
			_swap(from, to);
			_sparse[from] = 0;
			_dense.pop_back();

			if (empty())
				return end();
			return position;
		}

		/**
		 * @brief Removes a key from the container and returns the outcome
		 * of the operation.
		 * @return True if key was removed from the container.
		 */
		bool remove(key_type key)
		{
			if (valid(key) && count(key))
			{
				auto other = _dense.back();
				_swap(key, other);
				_sparse[key] = 0;
				_dense.pop_back();
				return true;
			}
			return false;
		}


		/**
		 * @brief Sorts the underlying dense array in ascending order. Calls
		 * sort(std::less).
		 */
		void sort()
		{
			sort(std::less<id_type>{});
		}

		/**
		 * @brief Sorts the underlying dense array given a predicate function.
		 *
		 * @tparam Pred Predicate function type.
		 * @param pred Predicate function.
		 *
		 * @todo Optimize by using sparse as the copy array, reducing space
		 * to O(1).
		 */
		template <typename Pred>
		void sort(Pred&& pred)
		{
			std::vector<Id_t> copy(size());
			std::iota(copy.begin(), copy.end(), 0);
			std::sort(copy.begin(), copy.end(),
				[&pred, this](const auto l, const auto r)
				{
					return pred(_dense[l], _dense[r]);
				});

			for (Id_t i = 0, len = static_cast<Id_t>(size()); i < len; ++i)
			{
				auto curr = i;
				auto next = copy[curr];

				while (curr != next)
				{
					_swap(_dense[copy[curr]], _dense[copy[next]]);
					copy[curr] = curr;
					curr = next;
					next = copy[curr];
				}
			}
		}

		/**
		 * @note LOOKUP
		 */

		 /**
		  * @brief Searches for the key in the container and returns an iterator
		  * to the found key. end() is returned if the key is not found.
		  */
		iterator find(key_type key) const
		{
			if (valid(key) && count(key))
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
		 * @brief Checks if key is in the container.
		 */
		bool contains(key_type key) const
		{
			return count(key);
		}


		/**
		 * @note SET OPERATIONS
		 */

		 /**
		  * @brief Set Equal operation.
		  * @returns True if all keys in this is contained in other.
		  */
		template<typename TOther>
		bool set_equal(const sparse_set<TOther>& other) const
		{
			if (size() != other.size())
				return false;

			for (auto id : other)
				if (!count(static_cast<key_type>(id)))
					return false;

			return true;
		}

		/**
		 * @brief Merge operation. Merges other into the container.
		 */
		template<typename TOther>
		void merge(const sparse_set<TOther>& other)
		{
			for (auto i : other)
				push_back(i);
		}

		/**
		 * @brief Intersect operation. Removes any key in container 
		 * not contained in other.
		 */
		template<typename TOther>
		void interect(const sparse_set<TOther>& other)
		{
			auto it = begin();
			while (it != end())
			{
				if (!other.count(*it))
					it = erase(*it);
				else
					++it;
			}
		}

		/**
		 * @note NON-MEMBER FUNCTIONS
		 */

		/**
		 * @brief Operator== overload. Compares keys orderwise within the
		 * container. See equal() for set equality comparison function.
		 */
		friend bool operator==(
			const sparse_set& lhs,
			const sparse_set& rhs)
		{
			return lhs._dense == rhs._dense;
		}

		/**
		 * @brief Swaps the contents of two sparse sets. The swap operation
		 * of two sparse_sets with different memory_resource is undefined.
		 */
		friend void swap(sparse_set& lhs, sparse_set& rhs) VECL_NOEXCEPT
		{
			lhs.swap(rhs);
		}


	private:

		void _swap(Id_t rhs, Id_t lhs)
		{
			auto from = _sparse[lhs];
			auto to = _sparse[rhs];

			std::swap(_sparse[lhs], _sparse[rhs]);
			std::swap(_dense[from], _dense[to]);
		}

		vector<Id_t> _dense;
		vector<Id_t> _sparse;
	};

	/**
	 * @brief Set Equal fold operation for sparse sets.
	 * @returns True if all sparse sets contain the same keys.
	 */
	template<typename Lhs, typename... Rhs>
	bool set_equal(
		const sparse_set<Lhs>& lhs,
		const Rhs&... rhs)
	{
		return (lhs.set_equal(rhs) && ...);
	}


}

#endif
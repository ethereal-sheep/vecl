#ifndef VECL_SPARSE_MAP_HPP
#define VECL_SPARSE_MAP_HPP

#ifndef VECL_SPARSE_SIZE
#define VECL_SPARSE_SIZE 1024
#endif

#include "config/config.h"
#include "concepts.hpp"

#include <memory_resource> // pmr::memory_resource
#include <vector> // pmr::vector
#include <algorithm> // swap
#include <tuple> // pair
#include <numeric> // iota

namespace vecl
{
	/**
	 * @brief A Sparse Map is a contiguous container that contains key-value
	 * pairs with strictly integral keys.
	 *
	 * Provides constant time insert, remove, and lookup, while providing
	 * locality of reference.
	 *
	 * Since the container is contiguous, the data is ordered as well.
	 * However, the ordering of data is not preserved when removing
	 * an element from the set.
	 *
	 * While the container allows random-access of its elements, the value
	 * of the key is important in maintaining the integrity of the container.
	 * As such, when a key or key-value pair is returned, the key component
	 * is either returned by value or const-reference.
	 *
	 * Therefore, algorithms like std::sort or std::remove cannot be used
	 * with the set. The container, however, does provide suitable functions
	 * to implement certain functions e.g. sparse_map::sort.
	 *
	 * @tparam Id Unsigned integer type.
	 * @tparam T Type of element.
	 */
	template <
		std::unsigned_integral Id,
		typename T>
		class sparse_map
	{
		/**
		 * @brief Iterator to adapt the container's separate dense
		 * and mapped array iterators into a single iterator.
		 */
		template <typename It1, typename It2, typename TPair>
		class _sparse_map_iterator
		{
			friend class sparse_map;

			/**
			 * @brief Wraps a reference into an interface where it can
			 * be called as a pointer.
			 */
			class _reference_wrapper_ptr
			{
				TPair wrapped;
			public:
				_reference_wrapper_ptr(TPair&& ref) : wrapped{ ref } {}
				VECL_NODISCARD TPair* operator->() { return &wrapped; }
				VECL_NODISCARD TPair operator*() { return wrapped; }
			};


			/**
			 * @brief Private constructor.
			 */
			_sparse_map_iterator(It1 i1, It2 i2) VECL_NOEXCEPT 
			: it1(i1), it2(i2) {}

			It1 it1;
			It2 it2;

		public:
			/**
			 * @note TYPE TRAITS
			 */
			using difference_type = ptrdiff_t;
			using value_type = TPair;
			using pointer = _reference_wrapper_ptr;
			using reference = TPair;
			using iterator_category = std::random_access_iterator_tag;

			_sparse_map_iterator() = delete;

			/** @brief Standard boiler plate for random-access iterators. */
			_sparse_map_iterator& operator=(const _sparse_map_iterator& rhs)
			{
				return it1 = rhs.it1, it2 = rhs.it2, *this;
			}

			/** @brief Standard boiler plate for random-access iterators. */
			_sparse_map_iterator& operator+=(size_t rhs)
			{
				return it1 += rhs, it2 += rhs, *this;
			}

			/** @brief Standard boiler plate for random-access iterators. */
			_sparse_map_iterator& operator-=(size_t rhs)
			{
				return it1 -= rhs, it2 -= rhs, *this;
			}

			/** @brief Standard boiler plate for random-access iterators. */
			_sparse_map_iterator& operator++()
			{
				return ++it1, ++it2, * this;
			}

			/** @brief Standard boiler plate for random-access iterators. */
			_sparse_map_iterator operator++(int)
			{
				_sparse_map_iterator orig = *this;
				return ++(*this), orig;
			}

			/** @brief Standard boiler plate for random-access iterators. */
			_sparse_map_iterator& operator--()
			{
				return --it1, --it2, * this;
			}

			/** @brief Standard boiler plate for random-access iterators. */
			_sparse_map_iterator operator--(int)
			{
				_sparse_map_iterator orig = *this;
				return operator--(), orig;
			}

			/** @brief Standard boiler plate for random-access iterators. */
			VECL_NODISCARD _sparse_map_iterator operator+(size_t rhs)
			{
				return _sparse_map_iterator(it1 + rhs, it2 + rhs);
			}

			/** @brief Standard boiler plate for random-access iterators. */
			VECL_NODISCARD _sparse_map_iterator operator-(size_t rhs)
			{
				return _sparse_map_iterator(it1 - rhs, it2 - rhs);
			}

			/** @brief Standard boiler plate for random-access iterators. */
			VECL_NODISCARD
			difference_type operator-(const _sparse_map_iterator& rhs)
			{
				return it1 - rhs.it1;
			}

			/** @brief Standard boiler plate for random-access iterators. */
			VECL_NODISCARD 
			bool operator==(const _sparse_map_iterator& rhs) const
			{
				return it1 == rhs.it1;
			}

			/** @brief Standard boiler plate for random-access iterators. */
			VECL_NODISCARD 
			bool operator!=(const _sparse_map_iterator& rhs) const
			{
				return !(*this == rhs);
			}

			/** @brief Standard boiler plate for random-access iterators. */
			VECL_NODISCARD 
			bool operator<(const _sparse_map_iterator& rhs) const
			{
				return it1 < rhs.it1;
			}

			/** @brief Standard boiler plate for random-access iterators. */
			VECL_NODISCARD 
			bool operator<=(const _sparse_map_iterator& rhs) const
			{
				return it1 <= rhs.it1;
			}

			/** @brief Standard boiler plate for random-access iterators. */
			VECL_NODISCARD 
			bool operator>(const _sparse_map_iterator& rhs) const
			{
				return it1 > rhs.it1;
			}

			/** @brief Standard boiler plate for random-access iterators. */
			VECL_NODISCARD 
			bool operator>=(const _sparse_map_iterator& rhs) const
			{
				return it1 >= rhs.it1;
			}

			/** @brief Standard boiler plate for random-access iterators. */
			VECL_NODISCARD pointer operator->() const
			{
				return pointer(std::make_pair(std::ref(*it1), std::ref(*it2)));
			}

			/** @brief Standard boiler plate for random-access iterators. */
			VECL_NODISCARD reference operator*() const
			{
				return std::make_pair(std::ref(*it1), std::ref(*it2));
			}
		};

	public:
		/**
		 * @note TYPE TRAITS
		 */
		using id_type = Id;
		using key_type = Id;
		using mapped_type = T;
		using value_type = std::pair<Id, T>;
		using reference = std::pair<const Id&, T&>;
		using const_reference = std::pair<const Id&, const T&>;
		using size_type = size_t;
		using difference_type = ptrdiff_t;

		using dense_array = std::pmr::vector<Id>;
		using mapped_array = std::pmr::vector<T>;
		using sparse_array = std::pmr::vector<Id>;
		using allocator_type = std::pmr::polymorphic_allocator<std::byte>;

		using iterator = _sparse_map_iterator<
			typename dense_array::iterator,
			typename mapped_array::iterator,
			reference>;

		using const_iterator = _sparse_map_iterator<
			typename dense_array::const_iterator,
			typename mapped_array::const_iterator,
			const_reference>;

		using reverse_iterator = _sparse_map_iterator<
			typename dense_array::reverse_iterator,
			typename mapped_array::reverse_iterator,
			reference>;

		using const_reverse_iterator = _sparse_map_iterator<
			typename dense_array::const_reverse_iterator,
			typename mapped_array::const_reverse_iterator,
			const_reference>;

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
			allocator_type mr = std::pmr::get_default_resource()
		) :
			_dense(mr), _mapped(mr), _sparse(VECL_SPARSE_SIZE, 0, mr)
		{
		}
		/**
		 * @brief Explicit Size Constructor. Explicitly sets the capacity of
		 * the container.
		 *
		 * @param capacity Capacity of the map.
		 * @param mr Pointer to a pmr resource. Default gets the default
		 * global pmr resource via get_default_resource().
		 */
		explicit sparse_map(
			size_type capacity,
			allocator_type mr = std::pmr::get_default_resource()
		) :
			_dense(mr), _mapped(mr), _sparse(capacity, 0, mr)
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
		template <std::input_iterator It>
		sparse_map(
			It first, It last,
			size_type capacity = VECL_SPARSE_SIZE,
			allocator_type mr = std::pmr::get_default_resource()
		) :
			_dense(mr), _mapped(mr), _sparse(capacity, 0, mr)
		{
			for (; first != last; ++first)
				push_back(*first);
		}

		/**
		 * @brief Initializer list Constructor.
		 *
		 * @param il Initializer list.
		 * @param capacity Capacity of the map.
		 * @param mr Pointer to a pmr resource. Default gets the default
		 * global pmr resource via get_default_resource().
		 */
		sparse_map(
			std::initializer_list<value_type> il,
			size_type capacity = VECL_SPARSE_SIZE,
			allocator_type mr = std::pmr::get_default_resource()
		) :
			sparse_map(il.begin(), il.end(), capacity, mr)
		{
		}

		/**
		 * @brief Default Copy Constructor. Uses same memory resource as
		 * other.
		 */
		sparse_map(const sparse_map& other) = default;

		/**
		 * @brief Memory-Extended Copy Constructor. Uses provided
		 * memory_resource to allocate copied arrays from other.
		 *
		 * @param other Const-reference to other.
		 * @param mr Pointer to a pmr resource.
		 */
		sparse_map(
			const sparse_map& other,
			allocator_type mr
		) :
			_dense(other._dense, mr),
			_mapped(other._mapped, mr),
			_sparse(other._sparse, mr)
		{
		}

		/**
		 * @brief Default Move Constructor. Constructs container with
		 * the contents of other using move-semantics. After the move, other
		 * is guaranteed to be empty.
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
		 * @param other Universal-reference to other.
		 * @param mr Pointer to a pmr resource.
		 */
		sparse_map(
			sparse_map&& other,
			allocator_type mr
		) :
			_dense(std::move(other._dense), mr),
			_mapped(std::move(other._mapped), mr),
			_sparse(std::move(other._sparse), mr)
		{

		}

		/**
		 * @brief Copy-Assignment Operator. Uses same memory resource as
		 * other. If the memory_resource of this is equal to that of other,
		 * the memory owned by this may be reused when possible.
		 */
		sparse_map& operator=(const sparse_map& other) = default;

		/**
		 * @brief Move-Assignment Operator. Uses same memory resource as
		 * other. If the memory_resource of this is equal to that of other,
		 * the memory owned by this may be reused when possible. Else, the
		 * assignment resolves to a copy-assignment.
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
		VECL_NODISCARD iterator begin() VECL_NOEXCEPT
		{
			return iterator(_dense.begin(), _mapped.begin());
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Iterator to end of range.
		 */
		VECL_NODISCARD iterator end() VECL_NOEXCEPT
		{
			return iterator(_dense.end(), _mapped.end());
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Iterator to beginning of range.
		 */
		VECL_NODISCARD const_iterator cbegin() const VECL_NOEXCEPT
		{
			return const_iterator(_dense.cbegin(), _mapped.cbegin());
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Iterator to end of range.
		 */
		VECL_NODISCARD const_iterator cend() const VECL_NOEXCEPT
		{
			return const_iterator(_dense.cend(), _mapped.cend());
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Reverse Iterator to beginning of range.
		 */
		VECL_NODISCARD reverse_iterator rbegin() VECL_NOEXCEPT
		{
			return reverse_iterator(_dense.rbegin(), _mapped.rbegin());
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Reverse Iterator to end of range.
		 */
		VECL_NODISCARD reverse_iterator rend() VECL_NOEXCEPT
		{
			return reverse_iterator(_dense.rend(), _mapped.rend());
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Reverse Iterator to beginning of range.
		 */
		VECL_NODISCARD const_reverse_iterator crbegin() const VECL_NOEXCEPT
		{
			return const_reverse_iterator(_dense.crbegin(), _mapped.crbegin());
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Reverse Iterator to end of range.
		 */
		VECL_NODISCARD const_reverse_iterator crend() const VECL_NOEXCEPT
		{
			return const_reverse_iterator(_dense.crend(), _mapped.crend());
		}


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
		 *  defined if and only if the maximum key value in the map is strictly
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
		VECL_NODISCARD bool empty() const VECL_NOEXCEPT 
		{ 
			return _dense.empty(); 
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
		 * @brief Constructs the object in-place at the end of the container
		 * with the given arguments, forwarded with std::forward<Args>(args...).
		 * Returns an (iterator, outcome) pair.
		 *
		 * (1) Key is inserted successfully;
		 * outcome is true and returning iterator points to newly created
		 * key-value pair.
		 * (2) Key is contained in container;
		 * outcome is false and returning iterator points to existing key-value
		 * pair.
		 * (3) Key value > max();
		 * outcome is false and returning iterator == end().
		 *
		 * @return (Iterator, Outcome) pair.
		 */
		template <typename...Args>
		std::pair<iterator, bool> emplace_back(key_type key, Args&&... args)
		{
			if (!valid(key))
				return std::make_pair(end(), false);

			if (!count(key))
			{
				_mapped.emplace_back(std::forward<Args>(args)...);
				_dense.emplace_back(key);
				_sparse[key] = static_cast<key_type>(_dense.size() - 1);
				return std::make_pair(--end(), true);
			}
			return std::make_pair(begin() + _sparse[key], false);
		}

		/**
		 * @brief Overload of emplace_back for key-value pair copy
		 * construction.
		 * Returns an (iterator, outcome) pair.
		 *
		 * (1) Key is inserted successfully;
		 * outcome is true and returning iterator points to newly created
		 * key-value pair.
		 * (2) Key is contained in container;
		 * outcome is false and returning iterator points to existing key-value
		 * pair.
		 * (3) Key value > max();
		 * outcome is false and returning iterator == end().
		 *
		 * @return (Iterator, Outcome) pair.
		 */
		std::pair<iterator, bool> emplace_back(const value_type& pair)
		{
			return insert(pair);
		}

		/**
		 * @brief Overload of emplace_back for key-value pair move
		 * construction.
		 * Returns an (iterator, outcome) pair.
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
		std::pair<iterator, bool> emplace_back(value_type&& pair)
		{
			return insert(std::move(pair));
		}

		/**
		 * @brief Appends the key and value to the end of the container by
		 * copy construction.
		 * If key is invalid or exists in the container, nothing happens.
		 */
		void push_back(key_type key, const mapped_type& value)
		{
			if (!valid(key)) return;

			if (!count(key))
			{
				_mapped.emplace_back(value);
				_dense.emplace_back(key);
				_sparse[key] = static_cast<key_type>(_dense.size() - 1);
			}
		}

		/**
		 * @brief Appends the key and value to the end of the container by move
		 * construction.
		 * If key is invalid or exists in the container, nothing happens.
		 */
		void push_back(key_type key, mapped_type&& value)
		{
			if (!valid(key)) return;

			if (!count(key))
			{
				_mapped.emplace_back(std::move(value));
				_dense.emplace_back(key);
				_sparse[key] = static_cast<key_type>(_dense.size() - 1);
			}
		}

		/**
		 * @brief Appends the pair to the end of the container by copy
		 * construction.
		 * If key is invalid or exists in the container, nothing happens.
		 */
		void push_back(const value_type& pair)
		{
			if (!valid(pair.first)) return;

			if (!count(pair.first))
			{
				_mapped.emplace_back(pair.second);
				_dense.emplace_back(pair.first);
				_sparse[pair.first] = static_cast<key_type>(_dense.size() - 1);
			}
		}

		/**
		 * @brief Appends the pair to the end of the container by move
		 * construction.
		 * If key is invalid or exists in the container, nothing happens.
		 */
		void push_back(value_type&& pair)
		{
			if (!valid(pair.first)) return;

			if (!count(pair.first))
			{
				auto key = pair.first;
				_mapped.emplace_back(std::move(pair.second));
				_dense.emplace_back(std::move(pair.first));
				_sparse[key] = static_cast<key_type>(_dense.size() - 1);
			}
		}

		/**
		 * @brief Removes the last key and value in the container.
		 * Calling pop_back on empty container is undefined.
		 */
		void pop_back()
		{
			_sparse[_dense.back()] = 0;
			_dense.pop_back();
			_mapped.pop_back();
		}

		/**
		 * @brief Inserts key into the container and returns an (iterator,
		 * outcome) pair.
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
		std::pair<iterator, bool> insert(const value_type& pair)
		{
			if (!valid(pair.first))
				return std::make_pair(end(), false);

			if (!count(pair.first))
			{
				_mapped.emplace_back(pair.second);
				_dense.emplace_back(pair.first);
				_sparse[pair.first] = static_cast<key_type>(_dense.size() - 1);
				return std::make_pair(--end(), true);
			}
			return std::make_pair(begin() + _sparse[pair.first], false);
		}

		/**
		 * @brief Inserts key into the container and returns an (iterator,
		 * outcome) pair.
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
		std::pair<iterator, bool> insert(value_type&& pair)
		{
			if (!valid(pair.first))
				return std::make_pair(end(), false);

			if (!count(pair.first))
			{
				auto key = pair.first;
				_mapped.emplace_back(std::move(pair.second));
				_dense.emplace_back(std::move(pair.first));
				_sparse[key] = static_cast<key_type>(_dense.size() - 1);
				return std::make_pair(--end(), true);
			}
			return std::make_pair(begin() + _sparse[pair.first], false);
		}


		/**
		 * @brief Erases a key-value from the container and returns an iterator
		 * to the replacing element.
		 * If the key is either
		 * invalid or not found in the container; or
		 * if the container is empty after the operation,
		 * end() is returned.
		 *
		 * @return Iterator to replacing element.
		 */
		iterator erase(key_type key)
		{
			if (count(key))
			{
				auto other = _dense.back();
				_swap(key, other);
				_sparse[key] = 0;
				_dense.pop_back();
				_mapped.pop_back();
				return begin() + _sparse[other];
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
			auto from = (*position).first;
			auto to = _dense.back();
			_swap(from, to);
			_sparse[from] = 0;
			_dense.pop_back();
			_mapped.pop_back();

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
			if (count(key))
			{
				auto other = _dense.back();
				_swap(key, other);
				_sparse[key] = 0;
				_dense.pop_back();
				_mapped.pop_back();
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
			std::vector<Id> copy(size());
			std::iota(copy.begin(), copy.end(), 0);
			std::sort(copy.begin(), copy.end(),
				[&pred, this](const auto l, const auto r)
				{
					return pred(_dense[l], _dense[r]);
				});

			for (Id i = 0, len = static_cast<Id>(size()); i < len; ++i)
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
		 * @brief Swaps the contents of two sparse sets. The swap operation
		 * of two sparse_sets with different memory_resource is undefined.
		 */
		void swap(sparse_map& x)
		{
			if (&x != this)
			{
				std::swap(_dense, x._dense);
				std::swap(_mapped, x._mapped);
				std::swap(_sparse, x._sparse);
			}
		}

		/**
		 * @note ELEMENT ACCESS
		 */
		 /**
		  * @brief Hashmap style subscript operator. Accesses specified
		  * element at key by reference if it exists, else an insertion
		  * is performed.
		  * 
		 * @throw std::out_of_range if key doesn't exist in the container.
		 */
		VECL_NODISCARD mapped_type& operator[](key_type key)
		{
			if (!valid(key))
				throw std::out_of_range("Sparse set key out of range!");
			
			if (!count(key))
			{
				_mapped.emplace_back();
				_dense.emplace_back(key);
				_sparse[key] = static_cast<key_type>(_dense.size() - 1);
				return _mapped.back();
			}
			return _mapped[_sparse[key]];
		}


		/**
		 * @brief Accesses specified element at key by reference with bounds
		 * checking.
		 *
		 * @throw std::out_of_range if key doesn't exist in the container.
		 */
		VECL_NODISCARD mapped_type& at(key_type key)
		{
			if (!count())
				throw std::out_of_range("Sparse set key out of range!");

			return _mapped[_sparse[key]];
		}

		/**
		 * @brief Accesses specified element at key by const-reference with
		 * bounds checking.
		 *
		 * @throw std::out_of_range if key doesn't exist in the container.
		 */
		VECL_NODISCARD const mapped_type& at(key_type key) const
		{
			if (!count())
				throw std::out_of_range("Sparse set key out of range!");

			return _mapped[_sparse[key]];
		}

		/**
		 * @brief Accesses specified element at key by const-pointer with
		 * bounds checking. Returns nullptr if index is out of range.
		 */
		VECL_NODISCARD mapped_type* at_if(key_type key) VECL_NOEXCEPT
		{
			if (!count())
				return nullptr;

			return &_mapped[_sparse[key]];
		}

		/**
		 * @brief Accesses specified element at key by const-reference with
		 * bounds checking. Returns nullptr if index is out of range.
		 */
		VECL_NODISCARD const mapped_type* at_if(key_type key) const VECL_NOEXCEPT
		{
			if (!count())
				return nullptr;

			return &_mapped[_sparse[key]];
		}

		/**
		 * @brief Accesses specified key at index by value with bounds
		 * checking.
		 * 
		 * @throw std::out_of_range if index is not within the range of the
		 * container.
		 */
		VECL_NODISCARD key_type key_at(size_t index)
		{
			return _dense.at(index);
		}

		/**
		 * @brief Accesses specified element at index by const-pointer with
		 * bounds checking. Returns nullptr if index is out of range.
		 */
		VECL_NODISCARD const key_type* key_at_if(size_t index) VECL_NOEXCEPT
		{
			if (index < size())
				return &_dense.at(index);
			return nullptr;
		}

		/**
		 * @brief Accesses first element by reference. Calling front on empty
		 * container is undefined.
		 */
		VECL_NODISCARD reference front()
		{
			return std::make_pair(std::ref(
				_dense.front()), std::ref(_mapped.front()));
		}
		/**
		 * @brief Accesses first element by const-reference. Calling front on empty
		 * container is undefined.
		 */
		VECL_NODISCARD const_reference front() const
		{
			return std::make_pair(
				std::ref(_dense.front()), std::ref(_mapped.front()));
		}

		/**
		 * @brief Accesses last element by reference. Calling back on empty
		 * container is undefined.
		 */
		VECL_NODISCARD reference back()
		{
			return std::make_pair(
				std::ref(_dense.back()), std::ref(_mapped.back()));
		}
		/**
		 * @brief Accesses last element by const-reference. Calling back on empty
		 * container is undefined.
		 */
		VECL_NODISCARD const_reference back() const
		{
			return std::make_pair(
				std::ref(_dense.back()), std::ref(_mapped.back()));
		}

		/**
		 * @brief Direct access to underlying dense array by const pointer.
		 */
		VECL_NODISCARD key_type* key_data() VECL_NOEXCEPT
		{
			return _dense.data();
		}
		/**
		 * @brief Direct access to underlying dense array by const pointer.
		 */
		VECL_NODISCARD const key_type* key_data() const VECL_NOEXCEPT
		{
			return _dense.data();
		}
		/**
		 * @brief Direct access to underlying dense array by const pointer.
		 */
		VECL_NODISCARD mapped_type* mapped_data() VECL_NOEXCEPT
		{
			return _mapped.data();
		}
		/**
		 * @brief Direct access to underlying dense array by const pointer.
		 */
		VECL_NODISCARD const mapped_type* mapped_data() const VECL_NOEXCEPT
		{
			return _mapped.data();
		}

		/**
		 * @note LOOKUP
		 */

		 /**
		  * @brief Searches for the key in the container and returns an iterator
		  * to the found key. If the key is not found, end() is returned.
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
		 * @note NON-MEMBER FUNCTIONS
		 */

		 /**
		  * @brief Operator== overload. Compares keys and elements orderwise
		  * within the container. 
		  */
		friend bool operator==(
			const sparse_map& lhs,
			const sparse_map& rhs
		)
		{
			return lhs._dense == rhs._dense && lhs._mapped == rhs._mapped;
		}

		/**
		 * @brief Operator!= overload. Compares keys and elements orderwise 
		 * within the container.
		 */
		friend bool operator!=(
			const sparse_map& lhs,
			const sparse_map& rhs
		)
		{
			return !(lhs == rhs);
		}

		/**
		 * @brief Swaps the contents of two sparse maps. The swap operation
		 * of two maps with different memory_resource is undefined.
		 */
		friend inline void swap(sparse_map& lhs, sparse_map& rhs) VECL_NOEXCEPT
		{
			lhs.swap(rhs);
		}

	private:
		void _swap(Id rhs, Id lhs)
		{
			auto from = _sparse[lhs];
			auto to = _sparse[rhs];

			std::swap(_sparse[lhs], _sparse[rhs]);
			std::swap(_dense[from], _dense[to]);
			std::swap(_mapped[from], _mapped[to]);
		}

		dense_array _dense;
		mapped_array _mapped;
		sparse_array _sparse;
	};
}

namespace std
{
	template<typename Id, typename T>
	inline void swap(
		vecl::sparse_map<Id, T>& lhs, 
		vecl::sparse_map<Id, T>& rhs
	) noexcept
	{
		lhs.swap(rhs);
	}
}

#endif
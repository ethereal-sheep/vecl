#ifndef VECL_SMALL_VECTOR_H
#define VECL_SMALL_VECTOR_H

#include "config/config.h"

#include <iterator> // advance, distance
#include <stdexcept> // length_error
#include <memory> // uninitialized_move, uninitialized_copy 

namespace vecl
{
	template<typename T, size_t N>
	struct small_vector_buffer
	{
		alignas(T) char _buffer[std::max(sizeof(size_t), N * sizeof(T))];
	};

	template <typename T>
	class small_vector_ref
	{
		struct _small_vec_ref_align_and_size 
		{
			alignas(small_vector_ref<T>) 
			char _ref[sizeof(small_vector_ref<T>)];
			alignas(T) char _first[sizeof(T)];
		};

		/**
		 * @brief Gets the first element in the stack buffer
		 */
		constexpr void* _get_first() const {
			return const_cast<void*>(reinterpret_cast<const void*>(
				reinterpret_cast<const char*>(this) +
				offsetof(_small_vec_ref_align_and_size, _first)));
		}

		/**
		 * @brief Destroys the elements in a given range.
		 */
		constexpr void _destroy_range(T* s, T* e) VECL_NOEXCEPT
		{
			while (s != e) (--e)->~value_type();
		}

		constexpr size_t _get_new_capacity_after_grow(size_t min_size)
		{
			// if requested min amount of ele exceeds max
			if (min_size > max_size())
				throw std::length_error(
					"min_size requested exceeds small_vector's max_size"
				);

			// if capacity already at max
			if(capacity() == max_size())
				throw std::length_error(
					"small_vector's capacity already at max_size"
				);

			// clamp between min and max so we have 
			// sufficient for min yet do not exceed max
			return std::min(std::max(min_size, 2 * capacity() + 1), max_size());
		}

		constexpr void* _allocate(size_t size) const
		{
			return malloc(size);
		}

		constexpr void _deallocate(void* ptr, [[maybe_unused]] size_t size) const
		{
			free(ptr);
		}

		constexpr T* _allocate_for_grow(size_t min_size, size_t& new_capacity)
		{
			new_capacity = _get_new_capacity_after_grow(min_size);
			return static_cast<T*>(_allocate(new_capacity * sizeof(T)));
		}

		constexpr void _take_allocation_for_grow(
			T* new_buffer, 
			size_t& new_capacity
		) 
		{
			_deallocate_current_buffer();

			_buffer = new_buffer;
			_capacity = new_capacity;
		}

		constexpr void _deallocate_current_buffer()
		{
			if (!is_small())
				_deallocate(_buffer, _capacity);
			else // we scribble our og capacity on the storage buffer
				*reinterpret_cast<size_t*>(_buffer) = _capacity;
		}

		constexpr void _move_for_grow(T* new_buffer)
		{
			std::uninitialized_move(begin(), end(), new_buffer);
			_destroy_range(begin(), end());
		}

		constexpr void _grow_and_assign(size_t ele_n, const T& ele) {

			size_t new_capacity;
			T* new_buffer = _allocate_for_grow(ele_n, new_capacity);
			std::uninitialized_fill_n(new_buffer, ele_n, ele);
			_destroy_range(begin(), end());
			_take_allocation_for_grow(new_buffer, new_capacity);
			_size = ele_n;
		}

		constexpr void _grow(size_t min_size = 0) 
		{
			size_t new_capacity;
			T* new_buffer = _allocate_for_grow(min_size, new_capacity);
			_move_for_grow(new_buffer);
			_take_allocation_for_grow(new_buffer, new_capacity);
		}

		constexpr const T* _reserve_for_param(size_t ele_n, const T& ele)
		{
			size_t new_size = size() + ele_n;
			if (new_size <= capacity())
				return &ele;

			if (_is_reference_in_buffer(&ele))
			{
				size_t ele_i = &ele - begin();
				_grow(new_size);
				return begin() + ele_i;
			}

			_grow(new_size);
			return &ele;
		}

		constexpr T* _reserve_for_param(size_t ele_n, T& ele)
		{
			size_t new_size = size() + ele_n;
			if (new_size <= capacity())
				return &ele;

			if (_is_reference_in_buffer(&ele))
			{
				size_t ele_i = &ele - begin();
				_grow(new_size);
				return begin() + ele_i;
			}

			_grow(new_size);
			return &ele;
		}

		template <typename... Args> 
		constexpr T& _grow_and_emplace_back(Args&&... args)
		{
			size_t new_capacity;
			T* new_buffer = _allocate_for_grow(0, new_capacity);
			::new ((void*)(new_buffer + size())) T(std::forward<Args>(args)...);
			_move_for_grow(new_buffer);
			_take_allocation_for_grow(new_buffer, new_capacity);
			++_size;
			return back();
		}

		/**
		 * @return true if ref is in the range.
		 */
		constexpr bool _is_reference_in_range(const T* ref, const T* from, const T* to) const
		{
			std::less<> less;
			return !less(ref, from) && less(ref, to);
		}

		/**
		 * @return true if ref is in the buffer.
		 */
		constexpr bool _is_reference_in_buffer(const T* ref) const
		{
			return _is_reference_in_range(ref, begin(), end());
		}

		/**
		 * @return true if range is in the buffer.
		 */
		constexpr bool _is_range_in_buffer(const T* from, const T* to) const
		{
			std::less<> less;
			return !less(from, begin()) && !less(to, from) &&
				!less(end(), to);
		}

		/**
		 * @return true if ref is not invalidated after a clear.
		 */
		constexpr bool _is_valid_after_clear(const T* ref) const
		{
			return !_is_reference_in_buffer(ref);
		}


		/**
		 * @return true if will grow after adding ele_n elements
		 */
		constexpr bool _will_grow(size_t ele_n = 1) const
		{
			return (ele_n + _size) > capacity();
		}

		/**
		 * @brief Asserts that a range is still valid after a clear.
		 */
		constexpr void _assert_valid_after_clear(const T* from, const T* to) const
		{
			if (from == to)	return;
			VECL_ASSERT(_is_valid_after_clear(from));
			VECL_ASSERT(_is_valid_after_clear(to));
		}

		/**
		 * @brief Overload to ignore assertions when It is not an iterator
		 * of the container.
		 */
		template <typename It>
		requires !std::is_same<std::remove_const_t<It>, T*>::value
			constexpr void _assert_valid_after_clear(It, It) const {}


	public:
		/**
		 * @note TYPE TRAITS
		 */
		using value_type = T;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = value_type*;
		using const_pointer = const value_type*;

		using iterator = pointer;
		using const_iterator = const iterator;
		using reverse_iterator = typename std::reverse_iterator<iterator>;
		using const_reverse_iterator = const reverse_iterator;

	
		/**
		 * @note MEMBER FUNCTIONS
		 */
	protected:
		explicit small_vector_ref(size_t N)
			: _buffer(_get_first()), _size(0), _capacity(N)
		{
		}

	public:
		 /**
		  * @brief Deleted Constructors.
		  */
		constexpr small_vector_ref() = delete;
		constexpr small_vector_ref(const small_vector_ref&) = delete;
		constexpr small_vector_ref(small_vector_ref&&) = delete;

		/**
		* @brief Destructor.
		*/
		constexpr ~small_vector_ref()
		{
			if (!is_small())
				_deallocate(_buffer, _capacity);
		}

		/**
		 * @brief Copy-Assignment Operator.
		 */
		constexpr small_vector_ref& operator=(const small_vector_ref& other)
		{
			if (&other == this) return *this;

			if (size() >= other.size())
			{
				auto new_end = std::copy(other.begin(), other.end(), begin());
				_destroy_range(new_end, end());
				_size = other.size();
				return *this;
			}

			if (capacity() < other.size())
			{
				clear();
				_grow(other.size());
				std::uninitialized_copy(other.begin(), other.end(), begin());

				_size = other.size();
				return *this;

			}
			
			std::copy(other.begin(), other.begin() + size(), begin());
			std::uninitialized_copy(
				other.begin() + size(), 
				other.end(), 
				begin() + size());

			_size = other.size();
			return *this;
		}

		/**
		 * @brief Move-Assignment Operator.
		 */
		constexpr small_vector_ref& operator=(small_vector_ref&& other) VECL_NOEXCEPT_IF(false)
		{
			if (&other == this) return *this;

			if (!other.is_small())
			{
				_destroy_range(begin(), end());
				_deallocate_current_buffer();
				_buffer = other._buffer;
				_size = other.size();
				_capacity = other.capacity();
				
				other._buffer = other._get_first();
				other._size = 0;
				// get back the og capacity from the stack storage
				other._capacity = *reinterpret_cast<size_t*>(other._buffer);

				return *this;
			}

			if (size() >= other.size())
			{
				auto new_end = std::copy(other.begin(), other.end(), begin());
				_destroy_range(new_end, end());
				
				_size = other.size();
				other.clear();
				return *this;
			}

			if (capacity() < other.size())
			{
				clear();
				_grow(other.size());
				std::uninitialized_move(other.begin(), other.end(), begin());
				
				_size = other.size();
				other.clear();
				return *this;
			}


			std::move(other.begin(), other.begin() + size(), begin());
			std::uninitialized_move(
				other.begin() + size(),
				other.end(),
				begin() + size());

			_size = other.size();
			other.clear();
			return *this;
		}

		/**
		 * @brief Fill assign.
		 *
		 * @param ele_n Number of elements
		 * @param value Element to fill by const-reference
		 */
		constexpr void assign(
			size_type ele_n,
			const value_type& value = value_type()
		)
		{
			if (ele_n > capacity()) {
				_grow_and_assign(ele_n, value);
				return;
			}

			std::fill_n(begin(), std::min(ele_n, size()), value);
			if (ele_n > size())
				std::uninitialized_fill_n(end(), ele_n - size(), value);
			else if (ele_n < size())
				_destroy_range(begin() + ele_n, end());

			_size = ele_n;
		}

		/**
		 * @brief Range assign.
		 *
		 * @tparam It Iterator type.
		 *
		 * @param from Iterator to start of range.
		 * @param to Iterator to end of range.
		 */
		template <std::input_iterator It>
		constexpr void assign(It from, It to)
		{
			_assert_valid_after_clear(from, to);
			clear();
			append(from, to);
		}
		/**
		 * @brief Initializer list assign.
		 *
		 * @param il Initializer list.
		 */
		constexpr void assign(std::initializer_list<T> il)
		{
			clear();
			append(il);
		}

		/**
		 * @brief Copy assign.
		 */
		constexpr void assign(const small_vector_ref& rhs)
		{
			assign(rhs.begin(), rhs.end());
		}

		/**
		 * @note ELEMENT ACCESS
		 */
		 /**
		  * @brief Accesses specified element at index by reference with
		  * bounds checking.
		  *
		  * @throw std::out_of_range if index is out of range.
		  */
		VECL_NODISCARD constexpr reference at(size_type i)
		{
			if (i >= size())
				throw std::out_of_range("index out of range");
			return (*this)[i];
		}

		/**
		 * @brief Accesses specified element at index by const-reference with
		 * bounds checking.
		 *
		 * @throw std::out_of_range if index is out of range.
		 */
		VECL_NODISCARD constexpr const_reference at(size_type i) const
		{
			if (i >= size())
				throw std::out_of_range("index out of range");
			return (*this)[i];
		}

		/**
		 * @brief Subscript operator.
		*/
		VECL_NODISCARD constexpr reference operator[](size_type i)
		{
			VECL_ASSERT(i < size());
			return begin()[i];
		}

		/**
		 * @brief Subscript operator.
		*/
		VECL_NODISCARD constexpr const_reference operator[](size_type i) const
		{
			VECL_ASSERT(i < size());
			return begin()[i];
		}

		/**
		 * @brief Accesses first element by reference.
		 */
		VECL_NODISCARD constexpr reference front()
		{
			VECL_ASSERT(!empty());
			return begin()[0];
		}

		/**
		 * @brief Accesses first element by const-reference.
		 */
		VECL_NODISCARD constexpr const_reference front() const
		{
			VECL_ASSERT(!empty());
			return begin()[0];
		}

		/**
		 * @brief Accesses last element by reference.
		 */
		VECL_NODISCARD constexpr reference back()
		{
			VECL_ASSERT(!empty());
			return end()[-1];
		}

		/**
		 * @brief Accesses last element by const-reference.
		 */
		VECL_NODISCARD constexpr const_reference back() const
		{
			VECL_ASSERT(!empty());
			return end()[-1];
		}
		/**
		 * @brief Direct access to underlying buffer by pointer.
		 */
		VECL_NODISCARD constexpr pointer data()
		{
			return pointer(begin());
		}

		/**
		 * @brief Direct access to underlying buffery by const pointer.
		 */
		VECL_NODISCARD constexpr const_pointer data() const
		{
			return const_pointer(begin());
		}


		/**
		 * @note ITERATORS
		 */

		 /**
		  * @brief Standard Iterable Object boilerplate.
		  * @return Iterator to beginning of range.
		  */
		VECL_NODISCARD constexpr iterator begin() VECL_NOEXCEPT
		{
			return (iterator)(_buffer);
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Iterator to end of range.
		 */
		VECL_NODISCARD constexpr iterator end() VECL_NOEXCEPT
		{
			return (iterator)(_buffer)+_size;
		}
		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Const-iterator to beginning of range.
		 */
		VECL_NODISCARD constexpr const_iterator begin() const VECL_NOEXCEPT
		{
			return (const_iterator)(_buffer);
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Const-iterator to end of range.
		 */
		VECL_NODISCARD constexpr const_iterator end() const VECL_NOEXCEPT
		{
			return (const_iterator)(_buffer)+_size;
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Const-iterator to beginning of range.
		 */
		VECL_NODISCARD constexpr const_iterator cbegin() const VECL_NOEXCEPT
		{
			return begin();
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Const-iterator to end of range.
		 */
		VECL_NODISCARD constexpr const_iterator cend() const VECL_NOEXCEPT
		{
			return end();
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Reverse Iterator to beginning of range.
		 */
		VECL_NODISCARD constexpr reverse_iterator rbegin() VECL_NOEXCEPT
		{
			return std::make_reverse_iterator(end());
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Reverse Iterator to end of range.
		 */
		VECL_NODISCARD constexpr reverse_iterator rend() VECL_NOEXCEPT
		{
			return std::make_reverse_iterator(begin());
		}
		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Reverse Const-iterator to beginning of range.
		 */
		VECL_NODISCARD
			constexpr const_reverse_iterator rbegin() const VECL_NOEXCEPT
		{
			return std::make_reverse_iterator(end());
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Reverse Const-iterator to end of range.
		 */
		VECL_NODISCARD
			constexpr const_reverse_iterator rend() const VECL_NOEXCEPT
		{
			return std::make_reverse_iterator(begin());
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Reverse Const-iterator to beginning of range.
		 */
		VECL_NODISCARD
			constexpr const_reverse_iterator crbegin() const VECL_NOEXCEPT
		{
			return std::make_reverse_iterator(end());
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Reverse Const-iterator to end of range.
		 */
		VECL_NODISCARD
			constexpr const_reverse_iterator crend() const VECL_NOEXCEPT
		{
			return std::make_reverse_iterator(begin());
		}

		/**
		 * @note CAPACITY
		 */

		 /**
		  * @brief Checks if the container is empty.
		  */
		VECL_NODISCARD constexpr bool empty() const VECL_NOEXCEPT
		{
			return _size == 0;
		}

		/**
		 * @return Size of container.
		 */
		VECL_NODISCARD constexpr size_type size() const VECL_NOEXCEPT
		{
			return _size;
		}

		/**
		 * @return Size of container in bytes.
		 */
		VECL_NODISCARD constexpr size_type size_in_bytes() const VECL_NOEXCEPT
		{
			return _size * sizeof(T);
		}

		/**
		 * @return Capacity of container.
		 */
		VECL_NODISCARD constexpr size_type capacity() const VECL_NOEXCEPT
		{
			return _capacity;
		}

		/**
		 * @return Capacity of container in bytes.
		 */
		VECL_NODISCARD constexpr size_type capacity_in_bytes() const VECL_NOEXCEPT
		{
			return _capacity * sizeof(T);
		}

		/**
		 * @return Max size of container.
		 */
		VECL_NODISCARD constexpr size_type max_size() const VECL_NOEXCEPT
		{
			return std::numeric_limits<size_type>::max();
		}

		constexpr void reserve(size_type N)
		{
			if (capacity() < N)
				_grow(N);
		}

		
		VECL_NODISCARD constexpr bool is_small() const 
		{ 
			return begin() == _get_first();
		}


		/**
		 * @note MODIFIERS
		 */
		 /**
		  * @brief Clears the container.
		  */
		constexpr void clear() VECL_NOEXCEPT
		{
			_destroy_range(begin(), end());
			_size = 0;
		}

		/**
		 * @brief Range append.
		 *
		 * @tparam It Iterator type.
		 *
		 * @param from Iterator to start of range.
		 * @param to Iterator to end of range.
		 */
		template <std::input_iterator It>
		constexpr void append(It from, It to)
		{
			size_type ele_n = std::distance(from, to);

			if constexpr (std::is_same<std::remove_const_t<It>, T*>::value)
			{
				if (_is_range_in_buffer(from, to) && _will_grow(ele_n))
				{
					// convert iterators to indexes
					size_t from_i = from - begin();
					size_t to_i = to - begin();
					
					reserve(size() + ele_n);

					//un-invalidate iterators
					auto from_n = begin() + from_i;
					auto to_n = begin() + to_i;

					std::uninitialized_copy(from_n, to_n, end());
					_size += ele_n;
				}
			}

			reserve(size() + ele_n);
			std::uninitialized_copy(from, from + ele_n, end());
			_size += ele_n;
		}

		/**
		 * @brief Fill append.
		 *
		 * @param ele_n Number of elements
		 * @param ele Element to fill by const-reference
		 */
		constexpr void append(
			size_type ele_n,
			const value_type& ele = value_type()
		)
		{
			const T* ele_p = _reserve_for_param(ele_n, ele);
			std::uninitialized_fill_n(end(), ele_n, *ele_p);
			_size += ele_n;
		}

		/**
		 * @brief Initializer list append.
		 *
		 * @param il Initializer list.
		 */
		constexpr void append(std::initializer_list<T> il)
		{
			append(il.begin(), il.end());
		}

		/**
		 * @brief Inserts ele before cit in the container.
		 *
		 * @param cit Const-iterator to insert before
		 * @param ele value_type by universal-reference
		 */
		constexpr iterator insert(const_iterator cit, value_type&& ele)
		{
			iterator it = const_cast<iterator>(cit);

			if (it == end())
			{
				push_back(std::move(ele));
				return end() - 1;
			}

			VECL_ASSERT(
				_is_reference_in_buffer(it),
				"insertion iterator is out of bounds"
			);

			size_t it_i = it - begin();
			T* ele_p = _reserve_for_param(1, ele);
			it = begin() + it_i;

			new ((void*)end()) T(std::move(back()));
			std::move_backward(it, end() - 1, end());
			++_size;

			if (_is_reference_in_range(ele_p, it, end()))
				++ele_p;

			new ((void*)it) value_type(std::move(*ele_p));

			return it;
		}

		/**
		 * @brief Inserts ele before cit in the container.
		 *
		 * @param cit Const-iterator to insert before
		 * @param ele value_type by const-reference
		 */
		constexpr iterator insert(const_iterator cit, const value_type& ele)
		{
			return insert(cit, value_type(ele));
		}

		/**
		 * @brief Inserts ele_n number of elements before cit in the container.
		 *
		 * @param cit Const-iterator to insert before
		 * @param ele_n Number of elements
		 * @param ele value_type by const-reference
		 */
		constexpr iterator insert(
			const_iterator cit,
			size_type ele_n,
			const value_type& ele
		)
		{
			iterator it = const_cast<iterator>(cit);

			if (it == end())
			{
				append(ele_n, ele);
				return it;
			}

			VECL_ASSERT(
				_is_reference_in_buffer(it),
				"insertion iterator is out of bounds"
			);

			size_t it_i = it - begin();
			const value_type* ele_p = _reserve_for_param(ele_n, ele);
			it = begin() + it_i;

			value_type* old = end();

			if ((size_type)std::distance(it, end()) >= ele_n)
			{
				append(std::make_move_iterator(end() - ele_n),
					std::make_move_iterator(end()));

				std::move_backward(it, old - ele_n, old);

				if (it <= ele_p && ele_p < end())
					std::advance(ele_p, ele_n);

				std::fill_n(it, ele_n, *ele_p);
				return it;
			}

			_size += ele_n;
			size_t overwrite_n = std::distance(it, old);
			std::uninitialized_move(it, old, end() - overwrite_n);

			if (it <= ele_p && ele_p < end())
				std::advance(ele_p, ele_n);

			std::fill_n(it, overwrite_n, *ele_p);

			std::uninitialized_fill_n(old, ele_n - overwrite_n, *ele_p);

			return it;
		}

		/**
		 * @brief Inserts range before cit in the container.
		 *
		 * @tparam It Iterator type.
		 *
		 * @param cit Const-iterator to insert before
		 * @param from Iterator to start of range.
		 * @param to Iterator to end of range.
		 *
		 * @TODO fails if It is const T* and a reference in range
		 * @TODO optimize for grow
		 */
		template <std::input_iterator It>
		constexpr iterator insert(const_iterator cit, It from, It to)
		{
			iterator it = const_cast<iterator>(cit);
			
			if (it == end())
			{
				append(from, to);
				return it;
			}

			VECL_ASSERT(
				_is_reference_in_buffer(it),
				"insertion iterator is out of bounds"
			);

			size_type ele_n = std::distance(from, to);

			size_t it_i = it - begin();
			const value_type* ele_p = _reserve_for_param(ele_n, *from);
			it = begin() + it_i;

			value_type* old = end();
			size_t overwrite_n = (size_type)std::distance(it, old);

			if constexpr (std::is_same<std::remove_const_t<It>, T*>::value)
			{
				if (_is_reference_in_range(it, from, to))
				{
					if (overwrite_n >= ele_n)
					{
						append(std::make_move_iterator(end() - ele_n),
							std::make_move_iterator(end()));

						value_type* last = std::move_backward(
							it, old - ele_n, old);
						value_type* mid = std::copy(&*from, it, it);
						std::copy(last, &*to + ele_n, mid);
						return it;
					}

					_size += ele_n;
					value_type* last = end() - overwrite_n;
					std::uninitialized_move(it, old, end() - overwrite_n);

					value_type* curr = it;
					for (; overwrite_n > 0; --overwrite_n, ++curr, ++from)
					{
						if (from == it)
							std::advance(from, ele_n);
						*curr = *from;
					}

					for (; curr != last; ++curr, ++from)
					{
						if (from == it)
							std::advance(from, ele_n);
						new ((void*)curr) value_type(*from);
					}

					return it;
				}
			}

			if (overwrite_n >= ele_n)
			{
				append(std::make_move_iterator(end() - ele_n),
					std::make_move_iterator(end()));

				// Copy the existing elements that get replaced.
				std::move_backward(it, old - ele_n, old);

				if (it <= ele_p && ele_p < end())
				{
					std::advance(from, ele_n);
					std::advance(to, ele_n);
				}

				std::copy(from, to, it);
				return it;
			}

			_size += ele_n;
			std::uninitialized_move(it, old, end() - overwrite_n);

			for (
				value_type* curr = it;
				overwrite_n > 0;
				--overwrite_n, ++curr, ++from
				)
				*curr = *from;

			std::uninitialized_copy(from, to, old);

			return it;
		}

		/**
		 * @brief Inserts initializer list before cit in the container.
		 *
		 * @param cit Const-iterator to insert before
		 * @param il Initializer list.
		 */
		constexpr iterator insert(
			const_iterator cit,
			std::initializer_list<T> il)
		{
			return insert(cit, il.begin(), il.end());
		}

		/**
		 * @brief In-place constructs value_type with args before cit in
		 * the container.
		 *
		 * @param cit Const-iterator to insert before
		 * @param args Variadic arguments
		 */
		template<typename... Args>
		constexpr iterator emplace(const_iterator cit, Args&&... args)
		{
			return insert(cit, value_type(std::forward<Args>(args)...));
		}

		/**
		 * @brief Erases the element at cit from the container.
		 *
		 * @param cit Const-iterator position
		 */
		constexpr iterator erase(const_iterator cit)
		{
			VECL_ASSERT(
				_is_reference_in_buffer(cit),
				"erase iterator is out of bounds"
			);

			iterator it = const_cast<iterator>(cit);

			std::move(it + 1, end(), it);
			pop_back();

			return it;
		}

		/**
		 * @brief Erases a range from the container.
		 *
		 * @param cfrom Const-iterator to start of range.
		 * @param cto Const-iterator to end of range.
		 */
		constexpr iterator erase(const_iterator cfrom, const_iterator cto)
		{
			VECL_ASSERT(
				_is_range_in_buffer(cfrom, cto),
				"erase iterator range is out of bounds"
			);

			iterator from = const_cast<iterator>(cfrom);
			iterator to = const_cast<iterator>(cto);

			iterator last = std::move(to, end(), from);

			_destroy_range(last, end());
			_size -= std::distance(from, to);

			return from;
		}

		/**
		 * @brief Appends value to the end of the container.
		 *
		 * @param ele value_type by universal-reference
		 */
		constexpr void push_back(value_type&& ele)
		{
			T* ele_p = _reserve_for_param(1, ele);
			new ((void*)end()) value_type(std::move(*ele_p));
			++_size;
		}

		/**
		 * @brief Appends value to the end of the container.
		 *
		 * @param ele value_type by const-reference
		 */
		constexpr void push_back(const value_type& ele)
		{
			const T* ele_p = _reserve_for_param(1, ele);
			new ((void*)end()) value_type(*ele_p);
			++_size;
		}

		/**
		 * @brief In-place constructs value_type with args at the end of
		 * the container.
		 *
		 * @param args Variadic arguments
		 */
		template<typename... Args>
		constexpr reference emplace_back(Args&&... args)
		{
			if (size() >= capacity())
				return _grow_and_emplace_back(std::forward<Args>(args)...);

			new ((void*)end()) value_type(std::forward<Args>(args)...);
			++_size;
			return back();
		}

		/**
		 * @brief Removes the last element in the container.
		 */
		constexpr void pop_back()
		{
			VECL_ASSERT(!empty());
			--_size;
			end()->~T();
		}

	private:
		void* _buffer;
		size_type _size, _capacity;
	};

	template<typename T, size_t N>
	class small_vector : public small_vector_ref<T>, small_vector_buffer<T, N>
	{
		using super = small_vector_ref<T>;
	
	public:
		/**
		 * @brief Default Constructor.
		 */
		small_vector() : super(N) {}

		/**
		* @brief Destructor.
		*/
		constexpr ~small_vector()
		{
			super::clear();
		}

		/**
		 * @brief Explicit Fill Constructor.
		 *
		 * @param ele_n Number of elements
		 * @param ele Element to fill by const-reference
		 * 
		 * @TODO fill is slow if T is POD-type and defaulted
		 */
		constexpr explicit small_vector(
			size_t ele_n,
			const T& ele = T()
		) : 
			super(N)
		{
			super::append(ele_n, ele);
		}

		/**
		 * @brief Range Constructor.
		 *
		 * @tparam It Iterator type.
		 *
		 * @param from Iterator to start of range.
		 * @param to Iterator to end of range.
		 */
		template <std::input_iterator It>
		constexpr explicit small_vector(It from, It to)
			: super(N)
		{
			super::append(from, to);
		}

		/**
		 * @brief Copy Constructor.
		 */
		constexpr small_vector(const small_vector& other)
			: super(N)
		{
			if (!other.empty())
				super::operator=(other);
		}

		/**
		 * @brief Move Constructor.
		 */
		constexpr small_vector(small_vector&& other) VECL_NOEXCEPT_IF(false)
			: super(N)
		{
			if (!other.empty())
				super::operator=(std::move(other));
		}

		/**
		 * @brief Ref Copy Constructor.
		 */
		constexpr small_vector(const small_vector_ref<T>& other)
			: super(N)
		{
			if (!other.empty())
				super::operator=(other);
		}

		/**
		 * @brief Ref Move Constructor.
		 */
		constexpr small_vector(small_vector_ref<T>&& other) VECL_NOEXCEPT_IF(false)
			: super(N)
		{
			if (!other.empty())
				super::operator=(std::move(other));
		}

		/**
		 * @brief Initializer list Constructor.
		 *
		 * @param il Initializer list.
		 */
		constexpr small_vector(std::initializer_list<T> il)
			: super(N)
		{
			super::assign(il);
		}

		/**
		 * @brief Copy-Assignment Operator.
		 */
		constexpr small_vector& operator=(const small_vector& other)
		{
			super::operator=(other);
			return *this;
		}

		/**
		 * @brief Move-Assignment Operator.
		 */
		constexpr small_vector& operator=(small_vector&& other) VECL_NOEXCEPT_IF(false)
		{
			super::operator=(std::move(other));
			return *this;
		}

		/**
		 * @brief Ref Copy-Assignment Operator.
		 */
		constexpr small_vector& operator=(const small_vector_ref<T>& other)
		{
			super::operator=(other);
			return *this;
		}

		/**
		 * @brief Ref Move-Assignment Operator.
		 */
		constexpr small_vector& operator=(small_vector_ref<T>&& other) VECL_NOEXCEPT_IF(false)
		{
			super::operator=(std::move(other));
			return *this;
		}

		/**
		 * @brief IL-Assignment Operator.
		 *
		 * @param il Initializer list.
		 */
		constexpr small_vector& operator=(std::initializer_list<T> il)
		{
			super::assign(il);
			return *this;
		}
	};
}
#endif
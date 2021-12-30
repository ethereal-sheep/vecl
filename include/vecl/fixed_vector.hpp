#ifndef VECL_FIXED_VECTOR_H
#define VECL_FIXED_VECTOR_H

#include "config/config.h"
#include <iterator> // advance, distance
#include <stdexcept> // length_error
#include <memory> // uninitialized_move, uninitialized_copy 

namespace vecl
{
	/**
	 * @brief A Fixed Vector is an array with vector-like interface. 
	 * 
	 * Since the buffer is always stored on stack, we can expect much faster
	 * operations with a vecl::fixed_vector as compared to a std::vector.
	 * 
	 * It is useful for when an upper bound for the size can be determined.
	 *
	 * @tparam T Type of element.
	 * @tparam N Size of container.
	 * @tparam Strict false for no throw on overflow
	 */
	template<typename T, size_t N, bool Strict = true>
	class fixed_vector 
	{
		/**
		 * @brief Destroys the elements in a given range.
		 */
		constexpr void _destroy_range(T* s, T* e) VECL_NOEXCEPT
		{
			if constexpr (!std::is_trivially_destructible<T>::value)
				while (s != e) (--e)->~value_type();
		}

		/**
		 * @return true if ref is in the range.
		 */
		constexpr bool _is_reference_in_range(
			const T* ref, 
			const T* from, 
			const T* to
		) const
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
		requires (!std::is_same<std::remove_const_t<It>, T*>::value)
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
		using const_iterator = const_pointer;
		using reverse_iterator = typename std::reverse_iterator<iterator>;
		using const_reverse_iterator = 
			typename std::reverse_iterator<const_iterator>;

		/**
		 * @note MEMBER FUNCTIONS
		 */
		/**
		 * @brief Default Constructor.
		 */
		constexpr fixed_vector() = default;

		 /**
		 * @brief Destructor.
		 */
		constexpr ~fixed_vector()
		{
			clear();
		}

		/**
		 * @brief Explicit Fill Constructor.
		 *
		 * @param ele_n Number of elements
		 * @param ele Element to fill by const-reference
		 */
		constexpr explicit fixed_vector(
			size_t ele_n,
			const value_type& ele
		)
		{
			append(ele_n, ele);
		}

		/**
		 * @brief Explicit Fill Constructor.
		 *
		 * @param ele_n Number of elements
		 * @param ele Element to fill by const-reference
		 */
		constexpr explicit fixed_vector(
			size_t ele_n
		)
		{
			append(ele_n);
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
		constexpr explicit fixed_vector(It from, It to)
		{
			append(from, to);
		}

		/**
		 * @brief Copy Constructor.
		 */
		constexpr fixed_vector(const fixed_vector& other)
		{
			operator=(other);
		}

		/**
		 * @brief Move Constructor.
		 */
		constexpr fixed_vector(fixed_vector&& other) VECL_NOEXCEPT
		{
			operator=(std::move(other));
		}

		/**
		 * @brief Initializer list Constructor.
		 *
		 * @param il Initializer list.
		 */
		constexpr fixed_vector(std::initializer_list<T> il)
		{
			assign(il);
		}

		/**
		 * @brief Copy-Assignment Operator.
		 */
		constexpr fixed_vector& operator=(const fixed_vector& other)
		{
			if (&other == this) return *this;

			assign(other.begin(), other.end());
			return *this;
		}

		/**
		 * @brief Move-Assignment Operator.
		 */
		constexpr fixed_vector& operator=(fixed_vector&& other) VECL_NOEXCEPT
		{
			// if this is other we do nothing
			if (&other == this) return *this;
			// else we just clear and move all the stuff from other
			clear();
			std::uninitialized_move(other.begin(), other.end(), end());
			_size = other.size();
			other._size = 0;
			return *this;
		}

		/**
		 * @brief IL-Assignment Operator.
		 *
		 * @param il Initializer list.
		 */
		constexpr fixed_vector& operator=(std::initializer_list<T> il)
		{
			assign(il);
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
			const value_type& value
		)
		{
			// size check
			if (ele_n > N) {
				if constexpr (Strict)
					throw std::length_error(
						"max_size exceeded in fixed_vector"
					);
				else return;
			}

			// just overwrite over the current elements
			std::fill_n(begin(), std::min(ele_n, size()), value);
			// and either construct more elements at the end
			if (ele_n > this->size())
				std::uninitialized_fill_n(end(), ele_n - size(), value);
			// or destroy elements at the end to have size() == ele_n
			else if (ele_n < this->size())
				_destroy_range(begin() + ele_n, end());

			// and update our size
			_size = ele_n;
		}

		/**
		 * @brief Fill assign.
		 *
		 * @param ele_n Number of elements
		 */
		constexpr void assign(
			size_type ele_n
		)
		{
			// size check
			if (ele_n > N) {
				if constexpr (Strict)
					throw std::length_error(
						"max_size exceeded in fixed_vector"
					);
				else return;
			}

			// else just overwrite over the current elements
			std::fill_n(begin(), std::min(ele_n, size()), value_type());
			// and either construct more elements at the end
			if (ele_n > this->size())
				std::uninitialized_default_construct_n(end(), ele_n - size());
			// or destroy elements at the end to have size() == ele_n
			else if (ele_n < this->size())
				_destroy_range(begin() + ele_n, end());

			// and update our size
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
		constexpr void assign(const fixed_vector& rhs)
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
			return (iterator)(_buffer) + _size;
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
			return (const_iterator)(_buffer) + _size;
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Const-iterator to beginning of range.
		 */
		VECL_NODISCARD constexpr iterator cbegin() const VECL_NOEXCEPT
		{
			return begin();
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Const-iterator to end of range.
		 */
		VECL_NODISCARD constexpr iterator cend() const VECL_NOEXCEPT
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
		  * @return Spare size of container.
		  */
		VECL_NODISCARD constexpr size_type spare() const VECL_NOEXCEPT
		{
			return N - _size;
		}

		 /**
		  * @return Spare size of container in bytes.
		  */
		VECL_NODISCARD constexpr size_type spare_in_bytes() const VECL_NOEXCEPT
		{
			return spare() * sizeof(T);
		}

		/**
		 * @return Max size of container.
		 */
		VECL_NODISCARD consteval size_type max_size() const VECL_NOEXCEPT
		{
			return N;
		}

		/**
		 * @return Max size of container in bytes.
		 */
		VECL_NODISCARD consteval size_t max_size_in_bytes() const VECL_NOEXCEPT
		{
			return N * sizeof(T);
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
			// size check
			if (ele_n > spare()) {
				if constexpr (Strict)
					throw std::length_error(
						"max_size exceeded in fixed_vector"
						);
				else ele_n = spare(); // if not strict we just fill spare
			}

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
			const value_type& ele
		)
		{
			// size check
			if (ele_n > spare()) {
				if constexpr (Strict)
					throw std::length_error(
						"max_size exceeded in fixed_vector"
					);
				else ele_n = spare(); // if not strict we just fill spare
			}
			std::uninitialized_fill_n(end(), ele_n, ele); 
			_size += ele_n;
		}

		/**
		 * @brief Default Fill append.
		 *
		 * @param ele_n Number of elements
		 */
		constexpr void append(size_type ele_n)
		{
			// size check
			if (ele_n > spare()) {
				if constexpr (Strict)
					throw std::length_error(
						"max_size exceeded in fixed_vector"
					);
				else ele_n = spare(); // if not strict we just fill spare
			}
			std::uninitialized_default_construct_n(end(), ele_n);
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
		 * 
		 * @TODO fails if universal_ref is reference to buffer
		 */
		constexpr iterator insert(const_iterator cit, value_type&& ele)
		{
			// we cast away the constness of our iterator since
			// it is ours and this is a non-const function
			iterator it = const_cast<iterator>(cit);
			// if it == end(), we can just push back
			if (it == end())
			{
				push_back(std::move(ele));
				return end() - 1;
			}

			VECL_ASSERT(
				_is_reference_in_buffer(it), 
				"insertion iterator is out of bounds"
				);

			// size check
			if (size() >= N)
			{
				if constexpr (Strict)
					throw std::length_error(
						"max_size exceeded in fixed_vector"
						);
				else return it;
			}

			// we move construct the back element to the uninitialized memory
			new ((void*)end()) T(std::move(back()));
			// then we can move assign the rest of them one step to the right
			std::move_backward(it, end() - 1, end());
			++_size;

			// finally, just assign the element in place
			*it = std::move(ele);

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
			// we cast away the constness of our iterator since
			// it is ours and this is a non-const function
			iterator it = const_cast<iterator>(cit);
			// if it == end(), we can append
			if (it == end()) 
			{ 
				append(ele_n, ele);
				return it;
			}

			VECL_ASSERT(
				_is_reference_in_buffer(it), 
				"insertion iterator is out of bounds"
				);

			// size check
			if (ele_n > spare()) 
			{
				if constexpr (Strict)
					throw std::length_error(
						"max_size exceeded in fixed_vector"
						);
				else ele_n = spare(); // if not strict we just fill spare
			}


			// save the end
			value_type* old = end();
			const value_type* ele_p = &ele;

			// if overwriting elements is >= to inserting elements
			if ((size_type)std::distance(it, end()) >= ele_n)
			{
				// we have the simple case of appending the last
				// ele_n number of elements to the end
				append(std::make_move_iterator(end() - ele_n),
					std::make_move_iterator(end()));
				// and moving the rest backwards accordingly
				std::move_backward(it, old - ele_n, old);

				// if our reference was in the range of the move
				// we adjust the position of the ptr
				if (it <= ele_p && ele_p < end())
					std::advance(ele_p, ele_n);
				// finally, just fill the elements in the new space
				std::fill_n(it, ele_n, *ele_p);
				return it;
			}

			// else we update the size here so we can use the new end()
			_size += ele_n;
			// overwrite n is the number of elements we need to move
			size_t overwrite_n = std::distance(it, old);

			// move last overwrite_n number of elements to uninitialized memory
			// we know it is definitely uninitialized since 
			// overwrite_n < ele_n
			std::uninitialized_move(it, old, end() - overwrite_n);

			// if our reference was in the range of the move
			// we adjust the position of the ptr
			if (it <= ele_p && ele_p < end())
				std::advance(ele_p, ele_n);

			// fill over existing elements
			// over uninitialized fill unfilled space
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
		 */
		template <std::input_iterator It>
		constexpr iterator insert(const_iterator cit, It from, It to)
		{
			// we cast away the constness of our iterator since
			// it is ours and this is a non-const function
			iterator it = const_cast<iterator>(cit);
			// if it == end(), we can append
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
			// size check
			if (ele_n > spare()) 
			{
				if constexpr (Strict)
					throw std::length_error(
						"max_size exceeded in fixed_vector"
						);
				else ele_n = spare(); // if not strict we just fill spare
			}


			// save the end
			value_type* old = end();
			const value_type* ele_p = &*from;
			size_t overwrite_n = (size_t)std::distance(it, old);
			
			// if It is of type T*, we have to check if the it is a reference
			// to the range
			if constexpr (std::is_same<std::remove_const_t<It>, T*>::value)
			{
				// if it does, then we have to take extra care when copying
				// since our range will be invalidated by the insert
				if (_is_reference_in_range(it, from, to))
				{
					// if overwriting elements is >= to inserting elements
					if (overwrite_n >= ele_n)
					{
						// we have the simple case of appending the last
						// ele_n number of elements to the end
						append(std::make_move_iterator(end() - ele_n),
							std::make_move_iterator(end()));

						// and moving the rest backwards accordingly
						value_type* last = std::move_backward(
							it, old - ele_n, old);
						// then we piece wise copy from both parts of the range
						// into the freed up space in the middle
						value_type* mid = std::copy(&*from, it, it);
						std::copy(last, &*to + ele_n, mid);
						return it;
					}

					// else we update the size here so we can use the new end()
					_size += ele_n;
					value_type* last = end() - overwrite_n;
					
					// move last overwrite_n number of elements to unintialized 
					// memory; we know it is definitely uninitialized since 
					// overwrite_n < ele_n
					std::uninitialized_move(it, old, end() - overwrite_n);

					// piecewise copy and move from both parts of the range
					// the freed up space in the middle is only partially 
					// filled
					value_type* curr = it;
					for (; overwrite_n > 0; --overwrite_n, ++curr, ++from)
					{
						// if we reach the start of the freed space
						// then the next element in range must be
						// ele_n away
						if (from == it)
							std::advance(from, ele_n);
						*curr = *from;
					}

					for (; curr != last; ++curr, ++from)
					{
						// if we reach the start of the freed space
						// then the next element in range must be
						// ele_n away
						if (from == it)
							std::advance(from, ele_n);
						new ((void*)curr) value_type(*from);
					}

					return it;
				}
			}
			
			// if overwriting elements is >= to inserting elements
			if (overwrite_n >= ele_n)
			{
				// we have the simple case of appending the last
				// ele_n number of elements to the end
				append(std::make_move_iterator(end() - ele_n),
						std::make_move_iterator(end()));

				// and moving the rest backwards accordingly
				std::move_backward(it, old - ele_n, old);
				// here we are guaranteed that the iterator is not in the range
				// but we still have to check if the range will be affected
				// by the move; if it is, adjust the positions of the range
				if (it <= ele_p && ele_p < end())
				{
					std::advance(from, ele_n);
					std::advance(to, ele_n);
				} 

				// then we can trivially copy them to the space in the
				// middle
				std::copy(from, to, it);
				return it;
			}

			// else we update the size here so we can use the new end()
			_size += ele_n;
			// move last overwrite_n number of elements to uninitialized memory
			// we know it is definitely uninitialized since 
			// overwrite_n < ele_n
			std::uninitialized_move(it, old, end() - overwrite_n);

			// we do not need to check if our reference is in the range since
			// by this point, it is impossible to be
			// so we can just copy over the existing elements
			for (
				value_type* curr = it; 
				overwrite_n > 0; 
				--overwrite_n, ++curr, ++from
				)
				*curr = *from;

			// and construct at the initialized memory
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

			// we cast away the constness of our iterator since
			// it is ours and this is a non-const function
			iterator it = const_cast<iterator>(cit);
			// just move one step to the left and pop the last
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

			// we cast away the constness of our range iterators since
			// they are ours and this is a non-const function
			iterator from = const_cast<iterator>(cfrom);
			iterator to = const_cast<iterator>(cto);
			
			// if they are the same don't erase
			if(from == to) return from;

			// just move left and destroy
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
			// size check
			if (size() >= N)
			{
				if constexpr (Strict)
					throw std::length_error(
						"max_size exceeded in fixed_vector"
						);
				else return;
			}

			new ((void*)end()) value_type(std::move(ele));
			++_size;
		}

		/**
		 * @brief Appends value to the end of the container.
		 * 
		 * @param ele value_type by const-reference
		 */
		constexpr void push_back(const value_type& ele)
		{
			push_back(value_type(ele));
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
			// size check
			if (size() >= N)
			{
				if constexpr (Strict)
					throw std::length_error(
						"max_size exceeded in fixed_vector"
						);
				else return back();
			}

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

		/**
		 * @brief Swaps the contents of two fixed_vectors.
		 */
		constexpr void swap(fixed_vector& other)
		{
			if (&other == this) return;

			// get the shared number of elements
			size_type shared_n = std::min(size(), other.size());
			// and swap only up to the shared number
			for (size_type i = 0; i < shared_n; ++i)
				std::swap(begin()[i], other.begin()[i]);

			// then depending on which is vector is bigger
			// we move the extra elements from the bigger vector to the
			// smaller vector, then update the size and capacity
			if (size() > other.size())
			{
				size_type diff = size() - other.size();
				std::uninitialized_move(begin() + shared_n, end(), other.end());
				other._size += diff;

				_destroy_range(begin() + shared_n, end());
				_size = shared_n;
			}
			else if (size() < other.size())
			{
				size_type diff = other.size() - size();
				std::uninitialized_move(other.begin() + shared_n, other.end(), end());
				_size += diff;

				_destroy_range(other.begin() + shared_n, other.end());
				other._size = shared_n;
			}
		}

		/**
		 * @brief Swaps the contents of two fixed_vectors.
		 */
		friend inline void swap(
			fixed_vector& lhs, 
			fixed_vector& rhs
		) VECL_NOEXCEPT
		{
			lhs.swap(rhs);
		}

	private:
		alignas(T) char _buffer[N * sizeof(T)];
		size_type _size = 0;
	};

	/**
	 * @brief Specialize for 0 sized vector and disallow it.
	 */
	template<typename T>
	class fixed_vector<T, 0>;

}


namespace std 
{
	/**
	 * @brief Swaps the contents of two fixed_vectors.
	 */
	template<typename T, size_t N, bool S>
	inline void swap(
		vecl::fixed_vector<T,N,S>& lhs, 
		vecl::fixed_vector<T,N,S>& rhs
	) noexcept
	{
		lhs.swap(rhs);
	}
}


#endif
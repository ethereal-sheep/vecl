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
			while (s != e) (--e)->~value_type();
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
			const value_type& ele = value_type()
		)
		{
			append(ele_n, ele);
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
			if (&other == this) return *this;

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
			const value_type& value = value_type()
		)
		{
			if (ele_n > N) {
				if constexpr (Strict)
					throw std::length_error(
						"max_size exceeded in fixed_vector"
						);
				else return;
			}

			std::fill_n(begin(), std::min(ele_n, size()), value);
			if (ele_n > this->size())
				std::uninitialized_fill_n(end(), ele_n - size(), value);
			else if (ele_n < this->size())
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
			if (ele_n > spare()) {
				if constexpr (Strict)
					throw std::length_error(
						"max_size exceeded in fixed_vector"
						);
				else ele_n = spare();
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
			const value_type& ele = value_type()
		)
		{
			if (ele_n > spare()) {
				if constexpr (Strict)
					throw std::length_error(
						"max_size exceeded in fixed_vector"
						);
				else ele_n = spare();
			}
			std::uninitialized_fill_n(end(), ele_n, ele);
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

			if (size() >= N)
			{
				if constexpr (Strict)
					throw std::length_error(
						"max_size exceeded in fixed_vector"
						);
				else return it;
			}

			new ((void*)end()) T(std::move(back()));
			std::move_backward(it, end() - 1, end());
			++_size;

			new ((void*)it) value_type(std::move(ele));

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

			if (ele_n > spare()) 
			{
				if constexpr (Strict)
					throw std::length_error(
						"max_size exceeded in fixed_vector"
						);
				else ele_n = spare();
			}


			value_type* old = end();
			const value_type* ele_p = &ele;

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
			if (ele_n > spare())
			{
				if constexpr (Strict)
					throw std::length_error(
						"max_size exceeded in fixed_vector"
						);
				else ele_n = spare();
			}


			value_type* old = end();
			const value_type* ele_p = &*from;
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
		constexpr void push_back(const value_type& value)
		{
			push_back(value_type(value));
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
		constexpr void swap(fixed_vector& x)
		{
			if (&x != this)
			{
				fixed_vector temp = std::move(x);
				x = std::move(*this);
				*this = std::move(temp);
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
#ifndef VECL_UNINITIALIZED_BUFFER_H
#define VECL_UNINITIALIZED_BUFFER_H

#include "config/config.h"
#include <iterator> // reverse_iterator
#include <stdexcept> // out_of_range
#include <memory> // uninitialized_fill_n
#include <cstring> // memcpy

#include <iostream>

namespace vecl
{
	template<typename T>
	class uninitialized_buffer
	{
	public:
		using size_type = size_t;
		using diff_type = ptrdiff_t;
		using value_type = T;
		using pointer = T*;
		using const_pointer = T*;
		using reference = T&;
		using const_reference = T&;
		using iterator = pointer;
		using const_iterator = const_pointer;
		using reverse_iterator = typename std::reverse_iterator<pointer>;
		using const_reverse_iterator = typename std::reverse_iterator<const_pointer>;

	private:

	public:
		constexpr uninitialized_buffer(size_type ele_n) : _buffer{ new T[ele_n] }, _size{ ele_n }
		{
			VECL_ASSERT(ele_n > 0);
		}

		constexpr uninitialized_buffer(const uninitialized_buffer& rhs) : _buffer{ new T[rhs._size] }, _size{ rhs._size }
		{
			
		}

		constexpr uninitialized_buffer(uninitialized_buffer&& rhs) noexcept : _buffer{ nullptr }, _size{ 0 }
		{
			operator=(std::move(rhs));
		}

		constexpr uninitialized_buffer& operator=(const uninitialized_buffer& rhs)
		{
			delete[] _buffer;

			_buffer = new T[rhs._size];
			_size = rhs.size();

			return *this;
		}

		constexpr uninitialized_buffer& operator=(uninitialized_buffer&& rhs) noexcept
		{
			std::swap(rhs._buffer, _buffer);
			std::swap(rhs._size, _size);

			return *this;
		}

		constexpr void swap(uninitialized_buffer& rhs) noexcept
		{
			std::swap(rhs._buffer, _buffer);
			std::swap(rhs._size, _size);
		}


		virtual ~uninitialized_buffer()
		{
			delete[] _buffer;
		}

		VECL_NODISCARD constexpr pointer get(size_t i)
		{
			if (i >= size())
				throw std::out_of_range("index out of range");
			return _buffer + i;
		}

		VECL_NODISCARD constexpr const_pointer get(size_t i) const
		{
			if (i >= size())
				throw std::out_of_range("index out of range");
			return _buffer + i;
		}

		VECL_NODISCARD constexpr pointer data() { return _buffer; }
		VECL_NODISCARD constexpr const_pointer data() const { return _buffer; }

		VECL_NODISCARD constexpr iterator begin() { return _buffer; }
		VECL_NODISCARD constexpr iterator end() { return _buffer + _size; }
		VECL_NODISCARD constexpr const_iterator begin() const { return _buffer; }
		VECL_NODISCARD constexpr const_iterator end() const { return _buffer + _size; }
		VECL_NODISCARD constexpr const_iterator cbegin() const { return _buffer; }
		VECL_NODISCARD constexpr const_iterator cend() const { return _buffer + _size; }

		VECL_NODISCARD constexpr reverse_iterator rbegin() { return std::make_reverse_iterator(end()); }
		VECL_NODISCARD constexpr reverse_iterator rend() { return std::make_reverse_iterator(begin()); }
		VECL_NODISCARD constexpr const_reverse_iterator rbegin() const { return std::make_reverse_iterator(end()); }
		VECL_NODISCARD constexpr const_reverse_iterator rend() const { return std::make_reverse_iterator(begin()); }

		VECL_NODISCARD constexpr reference front() { return *begin(); }
		VECL_NODISCARD constexpr reference back() { return *(end() - 1); }
		VECL_NODISCARD constexpr const_reference front() const { return *begin(); }
		VECL_NODISCARD constexpr const_reference back() const { return *(end() - 1); }

		VECL_NODISCARD constexpr size_type size() const { return _size; }
		VECL_NODISCARD constexpr size_type size_in_bytes() const { return _size * sizeof(T); }
		VECL_NODISCARD constexpr size_type capacity() const { return _size; }
		VECL_NODISCARD constexpr size_type max_size() const { return _size; }

	private:
		T* _buffer;
		size_type _size;
	};


} // namespace vecl

namespace std
{
	template<typename T>
	void swap(vecl::uninitialized_buffer<T>& lhs, vecl::uninitialized_buffer<T>& rhs) noexcept
	{
		lhs.swap(rhs);
	}
} // namespace std

#endif
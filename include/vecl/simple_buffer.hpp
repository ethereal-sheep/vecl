#ifndef VECL_SIMPLE_BUFFER_H
#define VECL_SIMPLE_BUFFER_H

#include "config/config.h"
#include "uninitialized_buffer.hpp"
#include <memory> // uninitialized_fill_n

namespace vecl
{
	template<typename T>
	class simple_buffer : public uninitialized_buffer<T>
	{
		using super = uninitialized_buffer<T>;
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


		constexpr simple_buffer(size_type ele_n) : super(ele_n)
		{
			std::uninitialized_default_construct(super::begin(), super::end());
		}

		constexpr simple_buffer(size_type ele_n, const T& ele) : super(ele_n)
		{
			std::uninitialized_fill_n(super::begin(), ele_n, ele);
		}

		constexpr simple_buffer(const simple_buffer& rhs) : super(rhs)
		{
			if constexpr (std::is_trivially_copyable_v<T>)
				std::memcpy(super::begin(), rhs.begin(), rhs._size * sizeof(T));
			else
			{
				for (size_t i = 0; i < super::size(); ++i)
					std::construct_at(super::begin() + i, rhs[i]);
			}
		}
		
		constexpr simple_buffer(simple_buffer&& rhs) noexcept : super(std::move(rhs))
		{
			super::operator=(std::move(rhs));
		}

		constexpr simple_buffer& operator=(const simple_buffer& rhs)
		{
			super::operator=(rhs);

			if constexpr (std::is_trivially_copyable_v<T>)
				std::memcpy(super::begin(), rhs.begin(), rhs._size * sizeof(T));
			else
			{
				for (size_t i = 0; i < super::size(); ++i)
					std::construct_at(super::begin() + i, rhs[i]);
			}

			return *this;
		}

		constexpr simple_buffer& operator=(simple_buffer&& rhs) noexcept = default;

		~simple_buffer()
		{
			if constexpr (!std::is_trivially_destructible_v<T>)
				std::destroy(super::begin(), super::end());
		}

		VECL_NODISCARD constexpr reference at(size_type i)
		{
			if (i >= super::size())
				throw std::out_of_range("index out of range");
			return super::begin()[i];
		}

		VECL_NODISCARD constexpr const_reference at(size_type i) const
		{
			if (i >= super::size())
				throw std::out_of_range("index out of range");
			return super::begin()[i];
		}

		VECL_NODISCARD constexpr reference operator[](size_type i)
		{
			VECL_ASSERT(i < super::size());
			return super::begin()[i];
		}

		VECL_NODISCARD constexpr const_reference operator[](size_type i) const
		{
			VECL_ASSERT(i < super::size());
			return super::begin()[i];
		}
	};


} // namespace vecl

namespace std
{
	template<typename T>
	void swap(vecl::simple_buffer<T>& lhs, vecl::simple_buffer<T>& rhs) noexcept
	{
		lhs.swap(rhs);
	}
} // namespace std

#endif
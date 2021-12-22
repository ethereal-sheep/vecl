#ifndef VECL_ENUMERATE_HPP
#define VECL_ENUMERATE_HPP

#include "config/config.h"

#include "concepts.hpp"

namespace vecl
{
	template<std::integral T, typename R>
	class enumerable
	{
		T _start, _end;
		std::function<R(T)> _func;

		class _iterator
		{
			friend enumerable;

			T _it;
			const int _dir;
			const std::function<R(T)> _func;

			constexpr _iterator(T it, int dir, const std::function<R(T)>& func)
				: _it{ it }, _dir{ dir }, _func{ func } {}
		public:

			/** @brief type traits for iterator */
			using value_type = R;
			using pointer = const T*;
			using iterator_tag = std::bidirectional_iterator_tag;

			/** @brief Standard boiler plate for iterators. */
			constexpr _iterator& operator++()
			{
				return _it += _dir, * this;
			}

			/** @brief Standard boiler plate for iterators. */
			constexpr _iterator operator++(int)
			{
				_iterator orig = *this;
				return _it += _dir, orig;
			}

			/** @brief Standard boiler plate for iterators. */
			constexpr _iterator& operator--()
			{
				return _it -= _dir, * this;
			}

			/** @brief Standard boiler plate for iterators. */
			constexpr _iterator operator--(int)
			{
				_iterator orig = *this;
				return _it -= _dir, orig;
			}

			/** @brief Standard boiler plate for iterators. */
			constexpr _iterator& operator+=(int n)
			{
				return _it += n * _dir, *this;
			}

			/** @brief Standard boiler plate for iterators. */
			constexpr _iterator& operator-=(int n)
			{
				return _it -= n * _dir, *this;
			}

			/** @brief Standard boiler plate for iterators. */
			VECL_NODISCARD constexpr pointer operator->() const
			{
				return &_it;
			}

			/** @brief Standard boiler plate for iterators. */
			VECL_NODISCARD constexpr value_type operator*() const
			{
				return _func(_it);
			}

			/** @brief Standard boiler plate for iterators. */
			VECL_NODISCARD constexpr friend _iterator operator+(const _iterator& it, int n)
			{
				_iterator ret = it;
				return ret += n;
			}

			/** @brief Standard boiler plate for iterators. */
			VECL_NODISCARD constexpr friend _iterator operator-(const _iterator& it, int n)
			{
				_iterator ret = it;
				return ret -= n;
			}

			/** @brief Standard boiler plate for iterators. */
			VECL_NODISCARD constexpr bool operator==(const _iterator& rhs) const
			{
				return rhs._it == _it;
			}

			/** @brief Standard boiler plate for iterators. */
			VECL_NODISCARD constexpr std::partial_ordering operator<=>(const _iterator& it) const
			{
				if(_dir != it._dir) return std::partial_ordering::unordered;
				if (_dir < 0) return it._it <=> _it;
				return _it <=> it._it;
			}

		};

		enumerable(T start, T end, std::function<R(T)> func)
			: _start{ start }, _end{ end }, _func{ func } {}


		template<std::integral T>
		friend constexpr auto enumerate(T end)->enumerable<T, T>;

		template<std::integral T>
		friend constexpr auto enumerate(T start, T end)->enumerable<T, T>;

		template<std::integral T, typename Func>
		requires non_void_invocable<Func, T>
		friend constexpr auto enumerate(T end, Func func)->enumerable<T, std::invoke_result_t<Func, T>>;
		
		template<std::integral T, typename Func>
		requires non_void_invocable<Func, T>
		friend constexpr auto enumerate(T start, T end, Func func)->enumerable<T, std::invoke_result_t<Func, T>>;


	public:
		using value_type = T;
		using size_type = size_t;
		using pointer = value_type*;
		using const_pointer = const value_type*;

		using iterator = _iterator;


		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Iterator to beginning of range.
		 */
		VECL_NODISCARD constexpr iterator begin() VECL_NOEXCEPT
		{
			int dir = (_start <= _end) ? 1 : -1;
			return iterator(_start, dir, _func);
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Iterator to end of range.
		 */
		VECL_NODISCARD constexpr iterator end() VECL_NOEXCEPT
		{
			int dir = (_start <= _end) ? 1 : -1;
			return iterator(_end, dir, _func);
		}
	};


	template<std::integral T>
	VECL_NODISCARD constexpr auto enumerate(T end) -> enumerable<T,T>
	{
		return enumerable<T,T>(0, end, [](T p) constexpr -> T { return p; });
	}

	template<std::integral T>
	VECL_NODISCARD constexpr auto enumerate(T start, T end) -> enumerable<T, T>
	{
		return enumerable<T,T>(start, end, [](T p) constexpr -> T { return p; });
	}

	template<std::integral T, typename Func>
	requires non_void_invocable<Func, T>
	VECL_NODISCARD constexpr auto enumerate(T end, Func func) -> enumerable<T, std::invoke_result_t<Func, T>>
	{
		return enumerable<T, std::invoke_result_t<Func, T>>(0, end, func);
	}

	template<std::integral T, typename Func>
	requires non_void_invocable<Func, T>
	VECL_NODISCARD constexpr auto enumerate(T start, T end, Func func) -> enumerable<T, std::invoke_result_t<Func, T>>
	{
		return enumerable<T, std::invoke_result_t<Func, T>>(start, end, func);
	}
}

#endif
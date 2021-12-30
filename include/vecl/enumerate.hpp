#ifndef VECL_ENUMERATE_HPP
#define VECL_ENUMERATE_HPP

#include "config/config.h"
#include "concepts.hpp"
#include <functional> // function
#include <utility>

namespace vecl
{
	/**
	 * @brief An Enumerable is a helper object that allows
	 * for easy enumeration over a range through a range-based for-loop.
	 * 
	 * The object can only be created with the helper function enumerate().
	 *
	 * @tparam T Integral type to enumerate over
	 * @tparam R Return type of the enumeration
	 */
	template<std::integral T, typename R>
	class _enumerable_t
	{
		T _start, _end;
		std::function<R(T)> _func;

		/**
		 * @brief Iterator object.
		 */
		class _iterator
		{
			friend _enumerable_t;

			T _it;
			const int _dir;
			const std::function<R(T)> _func;

			/**
			 * @brief Private Constructor.
			 * Copy and move constructions and assignments are defaulted.
			 */
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
			VECL_NODISCARD constexpr 
			friend _iterator operator+(const _iterator& it, int n)
			{
				_iterator ret = it;
				return ret += n;
			}

			/** @brief Standard boiler plate for iterators. */
			VECL_NODISCARD 
			constexpr friend _iterator operator-(const _iterator& it, int n)
			{
				_iterator ret = it;
				return ret -= n;
			}

			/** @brief Standard boiler plate for iterators. */
			VECL_NODISCARD 
			constexpr bool operator==(const _iterator& rhs) const
			{
				return rhs._it == _it;
			}

			/** @brief Standard boiler plate for iterators. */
			VECL_NODISCARD 
			constexpr std::partial_ordering operator<=>(
				const _iterator& it
			) const
			{
				if(_dir != it._dir) return std::partial_ordering::unordered;
				if (_dir < 0) return it._it <=> _it;
				return _it <=> it._it;
			}

		};

	public:
		using value_type = R;
		using pointer = const T*;
		using iterator = _iterator;

		/**
		 * @brief Private Constructor. 
		 * Copy and move constructions and assignments are defaulted.
		 *
		 * @param start Start of range
		 * @param end End of range
		 * @param func Transform function called on each element in range
		 */
		_enumerable_t(T start, T end, std::function<R(T)> func)
			: _start{ start }, _end{ end }, _func{ std::move(func) } {}

		/**
		 * @note ITERATORS
		 */
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


	/**
	 * @brief Enumerates over a range from 0 to end. If end < 0, the
	 * enumeration goes in the -1 direction.
	 *
	 * @tparam T Integral type to enumerate over
	 *
	 * @param end End of range
	 *
	 * @return Enumerable object
	 */
	template<std::integral T>
	VECL_NODISCARD constexpr auto enumerate(T end) 
		-> _enumerable_t<T,T>
	{
		return _enumerable_t<T,T>(0, end, 
			[](T p) constexpr -> T { return p; });
	}

	/**
	 * @brief Enumerates over a range from start to end. If end < start, 
	 * the enumeration goes in the -1 direction.
	 *
	 * @tparam T Integral type to enumerate over
	 *
	 * @param start Start of range
	 * @param end End of range
	 *
	 * @return Enumerable object
	 */
	template<std::integral T>
	VECL_NODISCARD constexpr auto enumerate(T start, T end) 
		-> _enumerable_t<T, T>
	{
		return _enumerable_t<T,T>(start, end, 
			[](T p) constexpr -> T { return p; });
	}

	/**
	 * @brief Enumerates over a range from 0 to end and tranforms each 
	 * element in the range with a corresponding function.
	 *
	 * If end < 0, the enumeration goes in the -1 direction.
	 *
	 * @tparam T Integral type to enumerate over
	 * @tparam Func Non-void invocable type
	 *
	 * @param start Start of range
	 * @param end End of range
	 * @param func Transform function called on each element in range
	 *
	 * @return Enumerable object
	 */
	template<std::integral T, typename Func>
	requires non_void_invocable<Func, T>
	VECL_NODISCARD constexpr auto enumerate(T end, Func func) 
		-> _enumerable_t<T, std::invoke_result_t<Func, T>>
	{
		using R = std::invoke_result_t<Func, T>;
		return _enumerable_t<T, R>(0, end, func);
	}

	/**
	 * @brief Enumerates over a range from 0 to end and tranforms each 
	 * element in the range with a corresponding function.
	 *
	 * If end < 0, the enumeration goes in the -1 direction.
	 *
	 * @tparam T Integral type to enumerate over
	 * @tparam Func Non-void invocable type
	 *
	 * @param start Start of range
	 * @param end End of range
	 * @param func Transform function called on each element in range
	 *
	 * @return Enumerable object
	 */
	template<std::integral T, typename Func>
	requires non_void_invocable<Func, T>
	VECL_NODISCARD constexpr auto enumerate(T start, T end, Func func) 
		-> _enumerable_t<T, std::invoke_result_t<Func, T>>
	{
		using R = std::invoke_result_t<Func, T>;
		return _enumerable_t<T, R>(start, end, func);
	}
}

#endif
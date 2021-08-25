#ifndef VECL_SIMPLE_VIEW_HPP
#define VECL_SIMPLE_VIEW_HPP

#include "../config/config.h"
#include <iterator>

namespace vecl
{
	/**
	 * @brief An iterable object that allows for view access to an underlying
	 * range.
	 * 
	 * 
	 * 
	 * @tparam It iterator type of the underlying range
	 */
	template <typename It>
	class simple_view
	{

	public:
		/**
		 * @brief Constructs the simple_view.
		 * @param from Iterator to start of view
		 * @param to Iterator to end of view
		 */
		simple_view(It from, It to) : start(from), finish(to) {}

		/** @brief type traits for container */
		using value_type = typename It::value_type;
		using size_type = size_t;
		using difference_type = typename It::difference_type;

		using reference = typename It::reference;
		using pointer = typename It::pointer;
		using iterator = It;
		using iterator_category = std::forward_iterator_tag;


		/**
		 * @brief Standard boiler plate for iterable objects.
		 * @return Size of range
		 */
		VECL_NODISCARD size_type size() const VECL_NOEXCEPT
		{
			return finish - start;
		}
		/**
		 * @brief Standard boiler plate for iterable objects.
		 * @return True if range is empty
		 */
		VECL_NODISCARD bool empty() const VECL_NOEXCEPT
		{
			return !(size());
		}

		/**
		 * @brief Standard boiler plate for iterable objects.
		 * @return Iterator to the beginning of range
		 */
		VECL_NODISCARD iterator begin() const VECL_NOEXCEPT
		{
			return start;
		}

		/**
		 * @brief Standard boiler plate for iterable objects.
		 * @return Iterator to the end of range
		 */
		VECL_NODISCARD iterator end() const VECL_NOEXCEPT
		{
			return finish;
		}

		/**
		 * @brief Searches for the key in the view and returns an iterator
		 * to the found key. If the key is not found, end() is returned.
		 */
		VECL_NODISCARD iterator find(reference& element) const VECL_NOEXCEPT
		{
			for (auto it = begin(); it != end(); ++it)
				if (*it == element)
					return it;

			return end();
		}

	private:
		It start, finish;
	};

	/**
	 * @brief Returns a view of an iterable range.
	 * @tparam It It iterator type of the underlying range
	 * @param from Iterator to start of view
	 * @param to Iterator to end of view
	 * @return Constructed view object
	 */
	template <typename It>
	VECL_NODISCARD inline auto view(It&& from, It&& to) VECL_NOEXCEPT
	{
		return simple_view(
			std::forward<It>(from),
			std::forward<It>(to));
	}

	/**
	 * @brief Returns a view of an iterable range.
	 * @tparam Cont Type of iterable range
	 * @param cont Iterable range object
	 * @return Constructed view object
	 */
	template <typename Cont>
	VECL_NODISCARD inline auto view(Cont&& cont) VECL_NOEXCEPT
	{
		return simple_view(cont.begin(), cont.end());
	}

	/**
	 * @brief Returns a reversed view of an iterable range.
	 * @tparam Cont Type of iterable range
	 * @param cont Iterable range object
	 * @return Constructed view object
	 */
	template <typename Cont>
	VECL_NODISCARD inline auto reverse(Cont&& cont) VECL_NOEXCEPT
	{
		return simple_view(cont.rbegin(), cont.rend());
	}
}

#endif
#ifndef VECL_SKIP_VIEW_HPP
#define VECL_SKIP_VIEW_HPP

#include "../config/config.h"

namespace vecl
{
	/**
	 * @brief An iterable object that allows for view access to an underlying
	 * range.
	 * 
	 * 
	 * @tparam It iterator type of the underlying range.
	 * @tparam Pred Predicate function to determine if an element of view
	 * should be skipped. Predicate should return true if element should be skipped.
	 */
	template <typename It, typename Pred>
	class skip_view
	{
	public:
		/**
		 * @brief Constructs the skip_view
		 * @param from Iterator to start of view
		 * @param to Iterator to end of view
		 * @param func Predicate function object
		 */
		skip_view(It from, It to, Pred func) : start(from), finish(to), pred(func) {}

		/** @brief type traits for container */
		using predicate = Pred;
		using value_type = typename It::value_type;
		using size_type = size_t;
		using difference_type = typename It::difference_type;

		using reference = typename It::reference;
		using pointer = typename It::pointer;

		using iterator_category = std::forward_iterator_tag;

		/**
		 * @brief iterator type for the skip_view object
		 */
		class view_iterator final
		{
			friend class skip_view;

			/**
			 * @brief Check if curr is valid by checking the element against
			 * the predicate
			 * @return True or False
			 */
			bool valid() const {
				return !pred(*it);
			}

			/**
			 * @brief Constructs the view iterator.
			 * @param to End iterator from the parent class
			 * @param curr Curr iterator from the parent class
			 * @param func Predicate from the parent class
			 *
			 */
			view_iterator(It to, It curr, Pred func) :
				last{ to },
				it{ curr },
				pred{ func }
			{
				// if current isn't valid, iterate it until it is
				while (it != last && !valid()) {
					++(*this);
				}
			}

		public:
			/** @brief type traits for iterator */
			using difference_type = size_t;
			using value_type = skip_view::value_type;
			using reference = skip_view::reference;
			using pointer = skip_view::pointer;
			using iterator_category = skip_view::iterator_category;

			view_iterator() = delete;


			/** @brief Standard boiler plate for iterators but we do a check
			 * each time we change the position of iterator */
			view_iterator& operator++()
			{
				while (++it != last && !valid());
				return *this;
			}

			/** @brief Standard boiler plate for iterators but we do a check
			 * each time we change the position of iterator */
			view_iterator operator++(int)
			{
				view_iterator orig = *this;
				return ++(*this), orig;
			}

			/** @brief Standard boiler plate for iterators but we do a check
			 * each time we change the position of iterator */
			VECL_NODISCARD bool operator==(const view_iterator& rhs) const
			{
				return rhs.it == it;
			}

			/** @brief Standard boiler plate for iterators but we do a check
			 * each time we change the position of iterator */
			VECL_NODISCARD bool operator!=(const view_iterator& rhs) const
			{
				return !(*this == rhs);
			}

			/** @brief Standard boiler plate for iterators but we do a check
			 * each time we change the position of iterator */
			VECL_NODISCARD pointer operator->() const
			{
				return &*it;
			}

			/** @brief Standard boiler plate for iterators but we do a check
			 * each time we change the position of iterator */
			VECL_NODISCARD reference operator*() const
			{
				return *operator->();
			}

		private:
			It last;
			It it;
			Pred pred;
		};

		using iterator = view_iterator;
		using iterator_category = std::forward_iterator_tag;


		/**
		 * @brief Standard boiler plate for iterable objects. Size of
		 * skippable range cannot be found in constant time so returns
		 * a hint of the size.
		 */
		VECL_NODISCARD size_type size_hint() const VECL_NOEXCEPT
		{
			return end() - begin();
		}

		/**
		 * @brief Standard boiler plate for iterable objects
		 * @return Iterator to the beginning of range
		 */
		VECL_NODISCARD iterator begin() const VECL_NOEXCEPT
		{
			return iterator(finish, start, pred);
		}

		/**
		 * @brief Standard boiler plate for iterable objects
		 * @return Iterator to the end of range
		 */
		VECL_NODISCARD iterator end() const VECL_NOEXCEPT
		{
			return iterator(finish, finish, pred);
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
		Pred pred;
	};

	/**
	 * @brief Returns a skip_view of an iterable range.
	 * @tparam It It iterator type of the underlying range
	 * @tparam Pred Predicate function to determine if an element of view
	 * should be skipped. Predicate should return true if element should be skipped.
	 * @param from Iterator to start of view
	 * @param to Iterator to end of view
	 * @param pred Predicate function object. Return true if element should be skipped.
	 * @return Constructed view object
	 */
	template <typename It, typename Pred>
	VECL_NODISCARD inline auto skip(It&& begin, It&& end, Pred&& pred) VECL_NOEXCEPT
	{
		return skip_view(
			std::forward<It>(begin),
			std::forward<It>(end),
			std::forward<Pred>(pred));
	}

	/**
	 * @brief Returns a skip_view of an iterable range.
	 * @tparam Cont Type of iterable range
	 * @tparam Pred Predicate function to determine if an element of view
	 * should be skipped. Predicate should return true if element should be skipped.
	 * @param cont Iterable range object
	 * @param pred Predicate function object. Return true if element should be skipped.
	 * @return Constructed view object
	 */
	template <typename Cont, typename Pred>
	VECL_NODISCARD inline auto skip(Cont&& cont, Pred&& pred) VECL_NOEXCEPT
	{
		return skip_view(
			cont.begin(),
			cont.end(),
			std::forward<Pred>(pred));
	}


	/**
	 * @brief Returns a skip_view of an iterable range.
	 * @tparam It It iterator type of the underlying range
	 * @tparam Val Value type of droppable element
	 * @param from Iterator to start of view
	 * @param to Iterator to end of view
	 * @param val Element to be dropped from the view
	 * @return Constructed view object
	 */
	template <typename It, typename Val>
	VECL_NODISCARD inline auto drop(It&& begin, It&& end, Val&& val) VECL_NOEXCEPT
	{
		return skip_view(
			std::forward<It>(begin),
			std::forward<It>(end),
			[&val](const auto& i) { return i == val; });
	}

	/**
	 * @brief Returns a skip_view of an iterable range.
	 * @tparam Cont Type of iterable range
	 * @tparam Val Value type of droppable element
	 * @param cont Iterable range object
	 * @param val Element to be dropped from the view
	 * @return Constructed view object
	 */
	template <class Cont, typename Val>
	VECL_NODISCARD inline auto drop(Cont&& cont, Val&& val) VECL_NOEXCEPT
	{
		return skip_view(
			cont.begin(),
			cont.end(),
			[&val](const auto& i) { return i == val; });
	}

}
#endif
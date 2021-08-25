#ifndef VECL_SMALL_VECTOR_HPP
#define VECL_SMALL_VECTOR_HPP

#include "config/config.h"

#include <type_traits>
#include <vector>

namespace vecl
{

	/**
	 * @brief 
	 *
	 * @tparam T Type of element.
	 * @tparam N Small buffer size.
	 * @tparam Spill Allow spilling into heap
	 */
	template <typename T, size_t N, bool Spill = true>
	class small_vector
	{

	public:

		/**
		 * @note TYPE TRAITS
		 */
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using value_type = T;
		using reference = value_type&;
		using pointer = value_type*;
		using const_reference = const value_type&;
		using const_pointer = const value_type*;

		using allocator_type = typename std::pmr::vector<T>::allocator_type;

		using iterator = value_type*;
		using const_iterator = const iterator;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const iterator>;

		// members

		// element access

		// iterators

		// capacity

		// modifiers

		// non-member functions


	private:
		std::array<T, N> small;
		std::pmr::vector<T> vector;
	};
}


#endif
#ifndef VECL_CONCEPTS_HPP
#define VECL_CONCEPTS_HPP

#include "type_traits.hpp"

#include <concepts>
#include <utility>

namespace vecl
{
	template<typename C, typename V>
	concept insertable = requires(C & c, V && v) {
		c.insert(std::forward<V>(v));
	};

	template<typename C, typename V>
	concept back_insertable = requires(C & c, V && v) {
		c.push_back(std::forward<V>(v));
	};

	template <class F, class... Args >
	concept non_void_invocable = 
		std::regular_invocable<F, Args...> &&
		!std::is_same<void, std::invoke_result_t<F, Args...>>::value;


}

#endif
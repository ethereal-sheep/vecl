#ifndef PIPES_CONCEPTS_HPP
#define PIPES_CONCEPTS_HPP

#include <concepts>

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
}

#endif
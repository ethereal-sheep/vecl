#ifndef VECL_MEMORY_INTERNAL_HPP
#define VECL_MEMORY_INTERNAL_HPP

#include <memory_resource>

namespace vecl
{
	/**
	 * @brief Alias for std::pmr::memory_resource.
	 */
	using memory = std::pmr::memory_resource;
}
#endif
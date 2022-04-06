#ifndef VECL_CONDITIONAL_STORAGE_HPP

#include "config/config.h"

namespace vecl
{
	template<typename T, bool>
	struct conditional_storage;

	template<typename T>
	struct conditional_storage<T, true> { T _member; };

	template<typename T>
	struct conditional_storage<T, false> {};

} // namespace vecl

#endif // !VECL_CONDITIONAL_STORAGE_HPP
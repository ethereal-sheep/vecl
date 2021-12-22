#ifndef VECL_CONFIG_H
#define VECL_CONFIG_H

#ifndef VECL_NOEXCEPT
#define VECL_NOEXCEPT noexcept
#endif

#ifndef VECL_NODISCARD
#define VECL_NODISCARD [[nodiscard]]
#endif

#ifndef VECL_LIKELY
#define VECL_LIKELY [[likely]]
#endif

#ifndef VECL_UNLIKELY
#define VECL_UNLIKELY [[unlikely]]
#endif

#ifdef VECL_DISABLE_ASSERT
#undef VECL_ASSERT
#define VECL_ASSERT(...) (void(0))
#elif !defined VECL_ASSERT
#include <cassert>
#define VECL_ASSERT(condition, ...) assert(condition)
#endif


#endif

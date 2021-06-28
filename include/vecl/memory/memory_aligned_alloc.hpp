#ifndef VECL_MEMORY_ALIGNED_ALLOC_HPP
#define VECL_MEMORY_ALIGNED_ALLOC_HPP

#include "../config/config.h"
#include "memory_internal.hpp"

#ifdef _MSC_VER
#include <malloc.h> // _aligned_malloc, _aligned_free
#else
#include <cstdlib> //aligned_alloc, free
#endif

namespace vecl
{
	/**
	 * @brief Memory Resource using implemention dependent aligned_alloc
	 * function for resource acquistion. Useful for over-aligned 
	 * allocations when there are stricter alignment requirements.
	 * Thread-safety of aligned_alloc is implementation dependent.
	 * std::aligned_alloc is guaranteed to be thread-safe.
	 */
	class memory_aligned_alloc : public memory
	{
	protected:
		void* do_allocate(
			size_t bytes,
			size_t alignment
		) override
		{
#ifdef _MSC_VER
			return _aligned_malloc(bytes, alignment);
#else
			return ::aligned_alloc(alignment, bytes);
#endif
		}
		void do_deallocate(
			void* ptr,
			[[maybe_unused]] size_t bytes,
			[[maybe_unused]] size_t alignment) override
		{
#ifdef _MSC_VER
			_aligned_free(ptr);
#else
			::free(ptr);
#endif
		}
		bool do_is_equal(const memory& other) const VECL_NOEXCEPT override
		{
			return this == &other;
		}
	};

	/**
	 * @brief Returns a pointer to a memory resource using
	 * std::alloc_aligned and std::free for resource acquistion.
	 * Useful for over-aligned allocations required by some libraries.
	 * Thread-safety of std::alloc_aligned is guaranteed.
	 */
	static inline memory* get_memory_aligned_alloc()
	{
		static memory_aligned_alloc resource;
		return &resource;
	}
}
#endif
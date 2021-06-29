#ifndef VECL_MEMORY_MALLOC_HPP
#define VECL_MEMORY_MALLOC_HPP

#include "../config/config.h"
#include "memory_internal.hpp"

namespace vecl
{
	/**
	 * @brief Memory Resource using std::malloc and std::free for
	 * resource acquistion.
	 * Thread-safety of malloc is guaranteed since c++11.
	 */
	class memory_malloc : public memory
	{
	protected:
		void* do_allocate(
			size_t bytes, 
			[[maybe_unused]] size_t alignment
		) override
		{
			void* ptr = std::malloc(bytes);
			if (!ptr) throw std::bad_alloc();
			return ptr;
		}
		void do_deallocate(
			void* ptr, 
			[[maybe_unused]] size_t bytes, 
			[[maybe_unused]] size_t alignment) override
		{
			std::free(ptr);
		}
		bool do_is_equal(const memory& other) const VECL_NOEXCEPT override
		{
			return this == &other;
		}
	};

	/**
	 * @brief Returns a pointer to a memory resource that uses
	 * std::malloc and std::free for resource acquisition.
	 * Thread-safety of std::malloc is guaranteed since c++11.
	 */
	static inline memory* get_memory_malloc()
	{
		static memory_malloc resource;
		return &resource;
	}
}
#endif
#ifndef VECL_MEMORY_DEBUG_HPP
#define VECL_MEMORY_DEBUG_HPP

#include "../config/config.h"
#include "vector.hpp"
#include <mutex>

namespace vecl
{
	using memory = std::pmr::memory_resource;

	enum struct memory_debug_code 
	{
		INFO,
		WARNING,
		OUT_OF_MEMORY,
		ABORT
	};

	struct memory_debug_callback
	{
		virtual ~memory_debug_callback() = 0 {}
		virtual void report(
			memory_debug_code code, 
			const char* msg
		) = 0;
	};

	class memory_debug : public memory
	{
		// magic number identifying memory allocated by this resource
		static constexpr size_t allocated_pattern = 0xCAFEF00D;

		// 2nd magic number written over other magic number upon deallocation
		static constexpr size_t deallocated_pattern = 0xDEADC0DE;

		static constexpr std::byte scribbled_byte{ 0xA1 }; // byte used to scribble
		// deallocated memory

		static constexpr std::byte padded_byte{ 0xC6 }; 
		// byte used to write over newly-allocated memory and padding

		static constexpr size_t padding_size = alignof(max_align_t);

		struct alignas(std::max_align_t) padding
		{
			// This struct just make 'Header' readable.
			std::byte _padding[padding_size];
		};

		struct header
		{
			size_t _magic_number;
			padding _padding;
		};

		union aligned_header
		{
			// Maximally-aligned raw buffer big enough for a Header.
			header m_object;
			max_align_t m_alignment;
		};

		constexpr size_t block_size(size_t bytes) 
		{ 
			return sizeof(header) + bytes + padding_size; 
		}

	public:
		memory_debug(
			std::string name = std::string{ "default_memory_debug" },
			memory* upstream = std::pmr::get_default_resource(),
			memory_debug_callback* callback = nullptr
		) : 
			_name{ name }, 
			_blocks(upstream),
			_upstream(upstream)
		{

		}

		~memory_debug()
		{

		}

		memory_debug(const memory_debug&) = delete;
		memory_debug& operator=(const memory_debug&) = delete;
		memory_debug(memory_debug&&) = delete;
		memory_debug& operator=(memory_debug&&) = delete;

		// get upstream resource
		const memory* upstream_resource() const VECL_NOEXCEPT
		{ 
			return _upstream;
		}

		std::string_view name() const VECL_NOEXCEPT 
		{ 
			return _name; 
		}
		bool verbose() const VECL_NOEXCEPT { return _verbose_flag; }
		bool strict() const VECL_NOEXCEPT { return _strict_flag; }

		void set_verbose(bool is_verbose) VECL_NOEXCEPT 
		{ 
			 _verbose_flag = is_verbose; 
		}

		// statistics for UI
		size_t bytes_allocated() const VECL_NOEXCEPT 
		{ 
			return _bytes_allocated; 
			}                         // total bytes allocated in lifetime
		size_t bytes_deallocated() const VECL_NOEXCEPT 
		{ 
			return _bytes_allocated - _bytes_outstanding; 
		} // total bytes deallocated in lifetime
		size_t bytes_outstanding() const VECL_NOEXCEPT 
		{ 
			return _bytes_outstanding; 
		}                     // current outstanding bytes
		size_t bytes_highwater() const VECL_NOEXCEPT 
		{ 
			return _bytes_highwater; 
		}						  // highest number of outstanding bytes
		size_t blocks_allocated() const VECL_NOEXCEPT 
		{ 
			return _blocks_allocated; 
		}                       // highest number of outstanding bytes
		size_t blocks_outstanding() const VECL_NOEXCEPT 
		{ 
			return _blocks.size(); 
		}                        // number of outstanding blocks of memory

		// history
		size_t last_allocated_num_bytes() const VECL_NOEXCEPT 
		{ 
			return _last_allocated_num_bytes; 
		}
		size_t last_allocated_alignment() const VECL_NOEXCEPT 
		{ 
			return _last_allocated_alignment; 
		}
		void* last_allocated_address() const VECL_NOEXCEPT 
		{ 
			return _last_allocated_address; 
		}

		size_t last_deallocated_num_bytes() const VECL_NOEXCEPT 
		{ 
			return _last_deallocated_num_bytes; 
		}
		size_t last_deallocated_alignment() const VECL_NOEXCEPT 
		{ 
			return _last_deallocated_alignment; 
		}
		void* last_deallocated_address() const VECL_NOEXCEPT 
		{ 
			return _last_deallocated_address; 
		}

		// errors
		size_t misc_errors() const VECL_NOEXCEPT 
		{ 
			return _misc_errors; 
		}
		size_t bad_deallocations() const VECL_NOEXCEPT 
		{ 
			return _bad_deallocations; 
		}

		// memory leak reporting
		static size_t leaked_bytes() 
		{ 
			return _s_leaked_bytes; 
		}
		static size_t leaked_blocks() 
		{ 
			return _s_leaked_blocks; 
		}

		static void clear_leaked()
		{
			_s_leaked_bytes = 0;
			_s_leaked_blocks = 0;
		}

	protected:
		[[nodiscard]] virtual void* do_allocate(
			std::size_t bytes, 
			std::size_t alignment) override
		{

		}
		virtual void do_deallocate(
			void* ptr, 
			std::size_t bytes, 
			std::size_t alignment) override
		{

		}
		virtual bool do_is_equal(
			const memory& other) const VECL_NOEXCEPT override
		{

		}

	private:
		// record all allocations and hold the results
		struct allocation_record
		{
			void* _ptr;
			size_t _index;
			size_t _bytes;
			size_t _alignment;
		};

		mutable std::mutex _mutex;

		// metadata
		std::string _name;

		// flags
		std::atomic_bool _verbose_flag{ false };
		std::atomic_bool _strict_flag{ false };

		// statistics
		std::atomic_size_t _bytes_allocated{ 0 };
		std::atomic_size_t _bytes_outstanding{ 0 };
		std::atomic_size_t _bytes_highwater{ 0 };
		std::atomic_size_t _blocks_allocated{ 0 };

		// history 
		std::atomic_size_t _last_allocated_num_bytes{ 0 };
		std::atomic_size_t _last_allocated_alignment{ 0 };
		std::atomic<void*> _last_allocated_address{ nullptr };

		std::atomic_size_t _last_deallocated_num_bytes{ 0 };
		std::atomic_size_t _last_deallocated_alignment{ 0 };
		std::atomic<void*> _last_deallocated_address{ nullptr };

		// errors
		std::atomic_size_t _misc_errors{ 0 };
		std::atomic_size_t _bad_deallocations{ 0 };

		std::pmr::unordered_map<
			void*, 
			allocation_record, 
			std::hash<void*>> _blocks;
		
		memory* _upstream;

		static std::atomic_size_t _s_leaked_bytes;
		static std::atomic_size_t _s_leaked_blocks;
	};
	
	std::atomic_size_t memory_debug::_s_leaked_bytes(0);
	std::atomic_size_t memory_debug::_s_leaked_blocks(0);
}

#endif
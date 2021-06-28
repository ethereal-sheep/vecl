#ifndef VECL_MEMORY_DEBUG_MT_HPP
#define VECL_MEMORY_DEBUG_MT_HPP

#include "../config/config.h"
#include "memory_internal.hpp"
#include "memory_debug_callback.hpp"

#include <mutex> // mutex
#include <cstring> // memset
#include <string> // string
#include <unordered_map> // pmr::unordered_map

namespace vecl
{
	/**
	 * @brief Thread-safe Debug Memory Resource that provides runtime memory
	 * information such as statistics, history, leaks, and errors.
	 * All operations are thread-safe if and only if upstream is thread-safe
	 * (memory_malloc, std::pmr::synchronized_pool_resource, etc.). Otherwise, 
	 * allocate/deallocate is undefined on multi-threaded applications.
	 */
	class memory_debug_mt : public memory
	{
		/**
		 * @brief Magic number identifying memory allocated by this resource.
		 */
		static constexpr size_t allocated_pattern = 0xD1CEFACED1CEFACE;

		/**
		 * @brief 2nd magic number written over other magic number upon 
		 * deallocation.
		 */
		static constexpr size_t deallocated_pattern = 0xFADEDBEDFADEDBED;

		/**
		 * @brief Byte value used to scribble over deallocated memory.
		 */
		static constexpr std::byte scribble_byte{ 0xF9 };

		/**
		 * @brief Byte value used to write over newly-allocated memory and 
		 * padding.
		 */
		static constexpr std::byte padded_byte{ 0x7A }; 

		/**
		 * @brief Size of padding.
		 */
		static constexpr size_t padding_size = alignof(max_align_t);

		/**
		 * @brief Helper struct for aligning memory in header. 
		 * Makes 'Header' readable.
		 */
		struct alignas(std::max_align_t) padding
		{
			std::byte _padding[padding_size];
		};

		/**
		 * @brief Header struct to store allocation info.
		 */
		struct header
		{
			size_t _magic_number;
			padding _padding;
		};

		/**
		 * @brief Maximally-aligned raw buffer big enough for a header.
		 */
		union aligned_header
		{
			header _object;
			max_align_t _alignment;
		};

		/**
		 * @brief Allocation record struct for each allocated block.
		 */
		struct allocation_record
		{
			void* _ptr;
			size_t _index;
			size_t _bytes;
			size_t _alignment;
		};

		/**
		 * @brief Size of block of memory. 
		 */
		constexpr size_t block_size(size_t bytes) 
		{ 
			return sizeof(aligned_header) + bytes + padding_size;
		}

	public:

		/**
		 * @note MEMBER FUNCTIONS
		 */

		/**
		 * @brief Constructor. Constructs the debug memory.
		 * 
		 * @param name Name of the debug memory.
		 * @param callback Callback function. Specify nullptr for no callbacks.
		 * @param upstream Pointer to an upstream resource.
		 * 
		 */
		explicit memory_debug_mt(
			const std::string& name = "Threadsafe Debug Memory" ,
			memory_debug_callback* callback = nullptr,
			memory* upstream = std::pmr::get_default_resource()
		) : 
			_name{ name, upstream },
			_callback(callback),
			_blocks(upstream),
			_upstream(upstream)
		{
		}

		/**
		 * @brief Destructor. Destroys the debug memory.
		 */
		~memory_debug_mt()
		{
			// If any blocks have not been released, report them as leaked
			_s_leaked_blocks += blocks_outstanding();

			// Reclaim blocks that would have been leaked
			for (auto& [ptr, record] : _blocks)
			{
				if (_callback)
					_callback->report(
						memory_debug_code::MEMORY_LEAK,
						_name.c_str(),
						record._ptr, record._bytes, record._alignment);

				aligned_header* head = 
					static_cast<aligned_header*>(record._ptr) - 1;

				_s_leaked_bytes += record._bytes;
				_upstream->deallocate(
					head, 
					block_size(record._bytes), record._alignment);
			}
		}

		// Copy Constructor deleted.
		memory_debug_mt(const memory_debug_mt&) = delete;
		// Copy Assignment deleted.
		memory_debug_mt& operator=(const memory_debug_mt&) = delete;
		// Move Constructor deleted.
		memory_debug_mt(memory_debug_mt&&) = delete;
		// Move Assignment deleted.
		memory_debug_mt& operator=(memory_debug_mt&&) = delete;

		/**
		 * @return Upstream resource pointer used by debug memory.
		 */
		const memory* upstream_resource() const VECL_NOEXCEPT
		{ 
			return _upstream;
		}

		/**
		 * @return Name of the memory.
		 */
		std::string_view name() const VECL_NOEXCEPT 
		{ 
			return _name; 
		}
		

		/**
		 * @note STATISTICS
		 */

		/**
		 * @return Total bytes allocated during lifetime.
		 */
		size_t bytes_allocated() const VECL_NOEXCEPT 
		{ 
			return _bytes_allocated; 
		}
		/**
		 * @return Total bytes deallocated during lifetime.
		 */
		size_t bytes_deallocated() const VECL_NOEXCEPT 
		{ 
			return _bytes_allocated - _bytes_outstanding; 
		}
		
		/**
		 * @return Current outstanding bytes.
		 */
		size_t bytes_outstanding() const VECL_NOEXCEPT 
		{ 
			return _bytes_outstanding; 
		}    
		
		/**
		 * @return Highest number of allocated bytes.
		 */
		size_t bytes_highwater() const VECL_NOEXCEPT 
		{ 
			return _bytes_highwater; 
		}

		/**
		 * @return Total blocks allocated during lifetime i.e. total
		 * allocations.
		 */
		size_t blocks_allocated() const VECL_NOEXCEPT 
		{ 
			return _blocks_allocated; 
		}   
		/**
		 * @return Current outstanding blocks.
		 */
		size_t blocks_outstanding() const VECL_NOEXCEPT 
		{ 
			return _blocks.size(); 
		}

		/**
		 * @note HISTORY
		 */

		/**
		 * @return Number of bytes allocated on the last allocation.
		 */
		size_t last_allocated_num_bytes() const VECL_NOEXCEPT 
		{ 
			return _last_allocated_num_bytes; 
		}
		/**
		 * @return Alignment of the last allocation.
		 */
		size_t last_allocated_alignment() const VECL_NOEXCEPT 
		{ 
			return _last_allocated_alignment; 
		}

		/**
		 * @return Address of the last allocation.
		 */
		const void* last_allocated_address() const VECL_NOEXCEPT 
		{ 
			return _last_allocated_address; 
		}

		/**
		 * @return Number of bytes deallocated on the last deallocation.
		 */
		size_t last_deallocated_num_bytes() const VECL_NOEXCEPT 
		{ 
			return _last_deallocated_num_bytes; 
		}

		/**
		 * @return Alignment of the last deallocation.
		 */
		size_t last_deallocated_alignment() const VECL_NOEXCEPT 
		{ 
			return _last_deallocated_alignment; 
		}

		/**
		 * @return Address of the last deallocation.
		 */
		const void* last_deallocated_address() const VECL_NOEXCEPT 
		{ 
			return _last_deallocated_address; 
		}

		/**
		 * @note ERRORS
		 */

		/**
		 * @return Number of miscellaneous errors occurred during lifetime.
		 */
		size_t misc_errors() const VECL_NOEXCEPT 
		{ 
			return _misc_errors; 
		}

		/**
		 * @return Number of bad deallocations occurred during lifetime.
		 */
		size_t bad_deallocations() const VECL_NOEXCEPT 
		{ 
			return _bad_deallocations; 
		}

		/**
		 * @return Number of bytes leaked in current runtime.
		 */
		static size_t leaked_bytes() 
		{ 
			return _s_leaked_bytes; 
		}
		/**
		 * @return Number of blocks leaked in current runtime.
		 */
		static size_t leaked_blocks() 
		{ 
			return _s_leaked_blocks; 
		}

		/**
		 * @brief Resets leaked bytes/blocks count.
		 */
		static void clear_leaked()
		{
			_s_leaked_bytes = 0;
			_s_leaked_blocks = 0;
		}

	protected:
		virtual void* do_allocate(
			size_t bytes, 
			size_t alignment) override
		{
			if (alignment > alignof(max_align_t))
			{
				// over-aligned memory allocation
				if (_callback)
					_callback->report(
						memory_debug_code::OVER_ALIGNED,
						_name.c_str(),
						nullptr, bytes, alignment,
						_blocks_allocated);
			}

			// allocates extra bytes for header and padding
			aligned_header* head;

			try
			{
				// undefined on multi-threaded applications
				// if upstream is not thread-safe
				head = static_cast<aligned_header*>(
					_upstream->allocate(block_size(bytes)));
				
				if (!head)
					throw std::bad_alloc();
			}
			catch (const std::exception& a)
			{
				// bad alloc
				if (_callback)
					_callback->report(
						memory_debug_code::OUT_OF_MEMORY,
						_name.c_str(),
						nullptr, bytes, alignment);

				throw a;
			}
			catch (...)
			{
				// unexpected
				if (_callback)
					_callback->report(
						memory_debug_code::UNEXPECTED,
						_name.c_str(),
						nullptr, bytes, alignment);
				throw;
			}

			void* user = head + 1;

			// write over head and tail
			std::memset((std::byte*)(head + 1) - padding_size,
				std::to_integer<unsigned char>(padded_byte), padding_size);
			std::memset((std::byte*)(head + 1) + bytes,
				std::to_integer<unsigned char>(padded_byte), padding_size);

			head->_object._magic_number = allocated_pattern;

			if (_blocks.count(user))
			{
				// memory has been assigned previously
				if (_callback)
					_callback->report(
						memory_debug_code::MEMORY_REALLOC,
						_name.c_str(),
						user, bytes, alignment);

				throw std::bad_alloc();
			}

			// increments
			_bytes_allocated += bytes;
			_bytes_outstanding += bytes;
			if (_bytes_outstanding > _bytes_highwater)
				_bytes_highwater.store(_bytes_outstanding);

			// set history
			_last_allocated_num_bytes = bytes;
			_last_allocated_alignment = alignment;
			_last_allocated_address = user;

			// record the allocation
			{
				const std::scoped_lock lock(_mutex);
				_blocks.emplace(
					user, 
					allocation_record{ 
						user, 
						_blocks_allocated++, 
						bytes, 
						alignment });
			}

			if (_callback)
				_callback->report(
					memory_debug_code::ALLOCATION,
					_name.c_str(),
					user, bytes, alignment,
					(_blocks_allocated - 1));

			return user;
		}
		
		virtual void do_deallocate(
			void* ptr, 
			size_t bytes, 
			size_t alignment) override
		{
			//deallocating nullptr
			if (ptr == nullptr)
			{
				if (_callback)
					_callback->report(
						memory_debug_code::NULL_DELETE,
						_name.c_str(),
						ptr, bytes, alignment);

				++_bad_deallocations;
				return;
			}

			aligned_header* head = static_cast<aligned_header*>(ptr) - 1;

			// find allocation record of ptr
			auto block = _blocks.find(ptr);

			if (block == _blocks.end())
			{
				// if not found
				// but the block looks like a deallocated block
				if (deallocated_pattern == head->_object._magic_number)
				{
					if (_callback)
						_callback->report(
							memory_debug_code::DOUBLE_DELETE,
							_name.c_str(),
							ptr, bytes, alignment);
				}
				// otherwise, invalid delete on some pointer
				else
				{
					if (_callback)
						_callback->report(
							memory_debug_code::INVALID_DELETE,
							_name.c_str(),
							ptr, bytes, alignment);

				}
				++_bad_deallocations;
				return;

			}

			if (allocated_pattern != head->_object._magic_number)
			{
				if (_callback)
					_callback->report(
						memory_debug_code::CORRUPTED_HEADER,
						_name.c_str(),
						block->second._ptr,
						block->second._bytes,
						block->second._alignment,
						head->_object._magic_number);

				++_misc_errors;;
			}
			else
			{
				// check for over/underrun

				// Check the padding before the segment. Go backwards so we will
				// report the trashed byte nearest the segment.
				std::byte* pcBegin = static_cast<std::byte*>(ptr) - 1;
				std::byte* pcEnd = 
					reinterpret_cast<std::byte*>(&head->_object._padding);

				for (std::byte* pc = pcBegin; pcEnd <= pc; --pc)
				{
					if (padded_byte != *pc)
					{
						// underrun
						if (_callback)
							_callback->report(
								memory_debug_code::UNDERRUN,
								_name.c_str(),
								block->second._ptr,
								block->second._bytes,
								block->second._alignment,
								static_cast<int>(pcBegin + 1 - pc));

						++_misc_errors;
						break;
					}
				}

				// Check the padding after the segment.
				pcBegin = static_cast<std::byte*>(ptr) + block->second._bytes;
				pcEnd = pcBegin + padding_size;
				for (std::byte* pc = pcBegin; pc < pcEnd; ++pc)
				{
					if (padded_byte != *pc)
					{
						// overrun
						if (_callback)
							_callback->report(
								memory_debug_code::OVERRUN,
								_name.c_str(),
								block->second._ptr,
								block->second._bytes,
								block->second._alignment,
								static_cast<int>(pc + 1 - pcBegin));

						++_misc_errors;
						break;
					}
				}

			}

			if (_callback)
				_callback->report(
					memory_debug_code::DEALLOCATION,
					_name.c_str(),
					block->second._ptr,
					block->second._bytes,
					block->second._alignment, 
					block->second._index
					);

			// set as dead
			head->_object._magic_number = deallocated_pattern;

			// set history
			_last_deallocated_num_bytes = bytes;
			_last_deallocated_alignment = alignment;
			_last_deallocated_address = ptr;

			// undefined on multi-threaded applications
			// if upstream is not thread-safe
			_upstream->deallocate(
				head, 
				block_size(block->second._bytes), block->second._alignment);

			{
				const std::scoped_lock lock(_mutex);
				_blocks.erase(block);
			}
			_bytes_outstanding -= bytes;
		}
		
		virtual bool do_is_equal(
			const memory& other) const VECL_NOEXCEPT override
		{
			return this == &other;
		}

	private:

		mutable std::mutex _mutex;

		std::pmr::string _name;
		memory_debug_callback* _callback;
		
		std::pmr::unordered_map<
			void*,
			allocation_record,
			std::hash<void*>> _blocks;

		memory* _upstream;

		std::atomic_size_t _bytes_allocated{ 0 };
		std::atomic_size_t _bytes_outstanding{ 0 };
		std::atomic_size_t _bytes_highwater{ 0 };
		std::atomic_size_t _blocks_allocated{ 0 };

		std::atomic_size_t _last_allocated_num_bytes{ 0 };
		std::atomic_size_t _last_allocated_alignment{ 0 };
		std::atomic<void*> _last_allocated_address{ nullptr };

		std::atomic_size_t _last_deallocated_num_bytes{ 0 };
		std::atomic_size_t _last_deallocated_alignment{ 0 };
		std::atomic<void*> _last_deallocated_address{ nullptr };

		std::atomic_size_t _misc_errors{ 0 };
		std::atomic_size_t _bad_deallocations{ 0 };

		static std::atomic_size_t _s_leaked_bytes;
		static std::atomic_size_t _s_leaked_blocks;
	};
	
	std::atomic_size_t memory_debug_mt::_s_leaked_bytes(0);
	std::atomic_size_t memory_debug_mt::_s_leaked_blocks(0);
}

#endif
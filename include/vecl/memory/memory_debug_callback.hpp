#ifndef VECL_MEMORY_DEBUG_CALLBACK_HPP
#define VECL_MEMORY_DEBUG_CALLBACK_HPP

#include <iostream>

namespace vecl
{
	enum struct memory_debug_code
	{
		/**
		 * @brief An allocation.
		 * @param level INFO
		 * @param additional Index of allocation
		 */
		ALLOCATION,
		/**
		 * @brief An allocation.
		 * @param level INFO
		 * @param additional Index of allocation
		 */
		DEALLOCATION,
		/**
		 * @brief Overaligned block allocated.
		 * @param level WARN
		 */
		OVER_ALIGNED,
		/**
		 * @brief Allocated memory block has been allocated again.
		 * @param level ERROR
		 * @param address Always nullptr
		 */
		MEMORY_REALLOC,
		/**
		 * @brief Upstream has ran out of memory.
		 * @param level ERROR
		 */
		OUT_OF_MEMORY,
		/**
		 * @brief Buffer underrun.
		 * @param level WARN
		 * @param address Always nullptr
		 * @param additional Number of underrun bytes.
		 */
		UNDERRUN,
		/**
		 * @brief Buffer overerrun.
		 * @param level WARN
		 * @param additional Number of overrun bytes.
		 */
		OVERRUN,
		/**
		 * @brief Double delete on memory block.
		 * @param level WARN
		 */
		DOUBLE_DELETE,
		/**
		 * @brief Deleting nullptr.
		 * @param level WARN
		 */
		NULL_DELETE,
		/**
		 * @brief Deleting invalid pointer i.e. pointer not owned
		 * by memory resource.
		 * @param level WARN
		 * @param address Always nullptr
		 */
		INVALID_DELETE,
		/**
		 * @brief Memory header has been corrupted.
		 * @param level WARN
		 * @param additional Magic number at memory header.
		 */
		CORRUPTED_HEADER,
		/**
		 * @brief Memory leaked.
		 * @param level WARN
		 */
		MEMORY_LEAK,
		/**
		 * @brief Unexpected exception thrown.
		 * @param level ERROR
		 */
		UNEXPECTED
	};

	/**
	 * @brief Interface class for memory debug callback classes.
	 * Interfaces a virtual destructor and a virtual report function.
	 */
	struct memory_debug_callback
	{
		/**
		 * @brief Report function that receives messages from a memory_debug
		 * class (memory_debug, memory_debug_mt).
		 *
		 * @param code Enum Code that specifies the type of message. See
		 * memory_debug_code for list of codes and warning levels.
		 * @param name Name of memory_debug.
		 * @param address Address of the allocating/deallocating pointer if
		 * any.
		 * @param bytes Number of bytes allocated/deallocated in operation.
		 * @param alignment Byte alignment in operation.
		 * @param additional Additional data provided by memory_debug.
		 *
		 */
		virtual void report(memory_debug_code code,
			const char* name,
			const void* address,
			size_t bytes,
			size_t alignment,
			size_t additional = 0
		) = 0;

		/**
		 * @brief Virtual Destructor.
		 */
		virtual ~memory_debug_callback() = 0 {}
	};

	/**
	 * @brief Debug Callback class for writing to std::cout. Not thread-safe.
	 * The vecl library does not provide a thread-safe implementation of
	 * cout_memory_debug_callback.
	 */
	struct cout_memory_debug_callback : public memory_debug_callback
	{
		/**
		 * @brief Report function that receives messages from a memory_debug
		 * class (memory_debug, memory_debug_mt).
		 *
		 * @param code Enum Code that specifies the type of message. See
		 * memory_debug_code for list of codes and warning levels.
		 * @param name Name of memory_debug.
		 * @param address Address of the allocating/deallocating pointer if
		 * any.
		 * @param bytes Number of bytes allocated/deallocated in operation.
		 * @param alignement Byte alignment in operation.
		 * @param additional Additional data provided by memory_debug.
		 *
		 */
		void report(
			memory_debug_code code,
			const char* name,
			const void* address,
			size_t bytes,
			size_t alignment,
			size_t additional = 0
		)
		{
			switch (code)
			{
			case vecl::memory_debug_code::ALLOCATION:
				std::cout << name << "[" << additional << "]"
					<< ": Allocating " << bytes 
					<< " bytes at 0x" << address << std::endl;
				break;
			case vecl::memory_debug_code::DEALLOCATION:
				std::cout << name << "[" << additional << "]"
					<< ": Deallocating " << bytes 
					<< " bytes at 0x" << address << std::endl;
				break;
			case vecl::memory_debug_code::OVER_ALIGNED:
				std::cout << "*** Overaligned Memory Allocation by [" << additional 
					<< "]: Alignment of " << alignment 
					<< " bytes requested ***" << std::endl;
				break;
			case vecl::memory_debug_code::MEMORY_REALLOC:
				break;
			case vecl::memory_debug_code::OUT_OF_MEMORY:
				std::cout <<
					"*** Not enough memory: Allocating " <<
					bytes << " bytes ***" << std::endl;
				break;
			case vecl::memory_debug_code::UNDERRUN:
				std::cout << "*** Corrupted Memory at 0x" << address << ": Buffer "
					<< "underrun by " << additional << " ***" << std::endl;
				break;
			case vecl::memory_debug_code::OVERRUN:
				std::cout << "*** Corrupted Memory at 0x" << address << ": Buffer " 
					<< "overrun by " << additional << " ***" << std::endl;
				break;
			case vecl::memory_debug_code::DOUBLE_DELETE:
				std::cout << 
					"*** Deallocating already deallocated memory at " << 
					address << " ***" << std::endl;
				break;
			case vecl::memory_debug_code::NULL_DELETE:
				std::cout <<
					"*** Deallocating memory from null address ***" << std::endl;
				break;
			case vecl::memory_debug_code::INVALID_DELETE:
				std::cout <<
					"*** Deallocating invalid address at " <<
					address << " ***" << std::endl;
				break;
			case vecl::memory_debug_code::CORRUPTED_HEADER:
				std::cout << "*** Corrupted Memory at 0x" << address 
					<< ": Invalid Magic Number " << std::hex << additional 
					<< " ***" << std::endl;
				break;
			case vecl::memory_debug_code::MEMORY_LEAK:
				std::cout << 
					"*** Memory Leak at 0x" << address << ": " 
					<< bytes << " bytes leaked ***" << std::endl;
				break;
			case vecl::memory_debug_code::UNEXPECTED:
				break;
			default:
				break;
			}
		}
	};

	/**
	 * @brief Uses cout_memory_debug_callback as default callback function. Note
	 * that cout_memory_debug_callback is not thread-safe. The vecl library does
	 * not provide a thread-safe implementation of cout_memory_debug_callback.
	 */
	static inline memory_debug_callback* get_default_memory_debug_callback()
	{
		static cout_memory_debug_callback _callback;
		return &_callback;
	}
}

#endif
#ifndef VECL_BROADCAST_H
#define VECL_BROADCAST_H

#include "config/config.h"

#include <functional>
#include <vector>

namespace vecl
{

	/**
	 * @brief A Broadcast is a smart event-handler that models a
	 * broadcast design pattern.
	 *
	 * The container allows functions to listen to the broadcast.
	 * The broadcast will invoke all functions that are listening
	 * to it when triggered.
	 *
	 * A token is given to the listener when they listen to the broadcast.
	 * It handles the lifetime of the listener within the container.
	 * When the token goes out of scope, the listener dies and
	 * will be automatically removed from the container.
	 *
	 * @tparam Callable Callable object type
	 */
	template<typename Callable>
	class broadcast
	{

	public:

		/** @brief type traits for container */
		using callback = Callable;
		using token = std::shared_ptr<callback>;
		using allocator_type = std::pmr::polymorphic_allocator<std::byte>;

		/**
		 * @note MEMBER FUNCTIONS
		 */

		 /**
		  * @brief Default Constructor.
		  *
		  * @param mr Pointer to a pmr resource. Default gets the default
		  * global pmr resource via get_default_resource().
		  */
		explicit broadcast(
			allocator_type mr = std::pmr::get_default_resource()
		) : _listeners(mr)
		{
		}

		/**
		 * @brief Default Copy Constructor. Uses same memory resource as
		 * other.
		 */
		broadcast(const broadcast& other) = default;


		/**
		 * @brief Memory-Extended Copy Constructor. Uses provided
		 * memory_resource to allocate copied arrays from other.
		 *
		 * @param other Const-reference to other.
		 * @param mr Pointer to a pmr resource.
		 */
		broadcast(
			const broadcast& other,
			allocator_type mr
		) :
			_listeners(other._listeners, mr)
		{
		}

		/**
		 * @brief Default Move Constructor. Constructs container with
		 * the contents of other using move-semantics. After the move, other
		 * is guaranteed to be empty.
		 */
		broadcast(broadcast&& other) = default;

		/**
		 * @brief Memory-Extended Move Constructor. If memory_resource used
		 * by other is not the same as memory_resource provided, the
		 * construction resolves to a Memory-Extended copy construction. In
		 * which case, other is not guranteed to be empty after the move.
		 *
		 * @param other Universal-reference to other.
		 * @param mr Pointer to a pmr resource.
		 */
		broadcast(
			broadcast&& other,
			allocator_type mr
		) : _listeners(std::move(other._listeners), mr)
		{
		}


		/**
		 * @brief Copy-Assignment Operator. Uses same memory resource as
		 * other. If the memory_resource of this is equal to that of other,
		 * the memory owned by this may be reused when possible.
		 *
		 * @param other Const-reference to other.
		 */
		broadcast& operator=(const broadcast& other) = default;


		/**
		 * @brief Move-Assignment Operator. Assigns contents of other using
		 * move-semantics. After the move, other is guaranteed to be empty.
		 *
		 * @param other Universal-reference to other.
		 */
		broadcast& operator=(broadcast&& other) = default;

		/**
		 * @return Copy of allocator_type object used by the container.
		 */
		VECL_NODISCARD allocator_type get_allocator() const VECL_NOEXCEPT
		{
			return _subs.get_allocator();
		}

		/**
		 * @note CAPACITY
		 */

		/**
		 * @return Number of listeners. 
		 * 
		 * @warning Size is only an approximation as there may be dead listeners
		 * who have not been cleaned up.
		 */
		VECL_NODISCARD auto size() const VECL_NOEXCEPT
		{
			return _listeners.size();
		}

		/**
		 * @brief Checks if the container is empty.
		 * 
		 * @warning Only an approximation as there may be dead listeners
		 * who have not been cleaned up.
		 */
		VECL_NODISCARD bool empty() const VECL_NOEXCEPT
		{
			return _dense.empty();
		}

		/**
		 * @note MODIFIERS
		 */

		 /**
		  * @brief Registers a function to listen to the broadcast.
		  *
		  * @param callable Callable object to be registered
		  *
		  * @return Listener token(shared_ptr). When the token goes out
		  * of scope, the listener is revoked.
		  */
		VECL_NODISCARD token listen(Callable&& callable)
		{
			token handle = std::make_shared<callback>(callable);

			_listeners.push_back(handle);

			return handle;
		}

		/**
		 * @brief Triggers a broadcast to all listeners.
		 *
		 * @tparam Args Variadic argument list
		 *
		 * @param args Variadic arguments to be passed to listeners.
		 */
		template<typename... Args>
		void trigger(Args&&... args)
		{
			for (auto weak : _listeners)
			{
				if (auto callable = weak.lock())
					(*callable)(std::forward<Args>(args)...);
			}

			_listeners.erase(
				std::remove_if(
					_listeners.begin(), _listeners.end(),
					[](auto weak) { return weak.expired(); }
			), _listeners.end());

		}

		/**
		 * @brief Swaps the contents of two broadcasts. The swap operation
		 * of two broadcasts with different memory_resource is undefined.
		 */
		friend void swap(broadcast& lhs, broadcast& rhs) VECL_NOEXCEPT
		{
			std::swap(lhs._subs, rhs._subs);
		}

	private:
		using weak = std::weak_ptr<callback>;
		using listeners = std::pmr::vector<weak>;

		listeners _listeners;
	};
}

#endif
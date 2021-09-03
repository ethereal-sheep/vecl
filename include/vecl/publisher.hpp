#ifndef VECL_PUBLISHER_H
#define VECL_PUBLISHER_H

#include "config/config.h"
#include "broadcast.hpp"

#include <type_traits> // is_base_of
#include <functional> // function
#include <unordered_map> // pmr::unordered_map
#include <vector> // pmr::vector

namespace vecl
{
	/**
	 * @brief Simple hook for messages. Publisher class can be used as-is with 
	 * messages inheriting from simple_message.
	 */
	struct simple_message
	{
		virtual ~simple_message() {}
	};

	/**
	 * @brief A Publisher is a smart event-handler that models a
	 * publisher-subscriber design pattern.
	 *
	 * The container allows functions to be subscribed to an message,
	 * and will invoke them when the message is published.
	 * 
	 * A token is given to the subscriber per subscription,
	 * which handles the lifetime of the subscription within the container.
	 * When the token goes out of scope, the subscription ends and
	 * will be automatically removed from the container.
	 *
	 * @tparam Base Base class that all messages will inherit from
	 */
	template<typename Base = simple_message>
	class publisher
	{

	public:

		/** @brief type traits for container */
		using callback = std::function<void(const Base&)>;
		using allocator_type = std::pmr::polymorphic_allocator<std::byte>;

		/**
		 * @brief Default Constructor.
		 *
		 * @param mr Pointer to a pmr resource. Default gets the default
		 * global pmr resource via get_default_resource().
		 */
		explicit publisher(
			allocator_type mr = std::pmr::get_default_resource()
		) : 
			_subs(mr),
			_queue(mr)
		{
		}

		/**
		 * @brief Default Copy Constructor. Uses same memory resource as
		 * other.
		 */
		publisher(const publisher& other) = default;


		/**
		 * @brief Memory-Extended Copy Constructor. Uses provided
		 * memory_resource to allocate copied arrays from other.
		 *
		 * @param other Const-reference to other.
		 * @param mr Pointer to a pmr resource.
		 */
		publisher(
			const publisher& other,
			allocator_type mr
		) :
			_sub(other._sub, mr),
			_queue(other._queue, mr)
		{
		}


		/**
		 * @brief Default Move Constructor. Constructs container with
		 * the contents of other using move-semantics. After the move, other
		 * is guaranteed to be empty.
		 */
		publisher(publisher&& other) = default;

		/**
		 * @brief Memory-Extended Move Constructor. If memory_resource used
		 * by other is not the same as memory_resource provided, the
		 * construction resolves to a Memory-Extended copy construction. In
		 * which case, other is not guranteed to be empty after the move.
		 *
		 * @param other Universal-reference to other.
		 * @param mr Pointer to a pmr resource.
		 */
		publisher(
			publisher&& other,
			allocator_type mr
		) : 
			_subs(std::move(other._subs), mr), 
			_queue(std::move(other._queue), mr)
		{
		}


		/**
		 * @brief Copy-Assignment Operator. Uses same memory resource as
		 * other. If the memory_resource of this is equal to that of other,
		 * the memory owned by this may be reused when possible.
		 *
		 * @param other Const-reference to other.
		 */
		publisher& operator=(const publisher& other) = default;


		/**
		 * @brief Move-Assignment Operator. Assigns contents of other using
		 * move-semantics. After the move, other is guaranteed to be empty.
		 *
		 * @param other Universal-reference to other.
		 */
		publisher& operator=(publisher&& other) = default;


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
		 * @return Number of subscriptions to Message.
		 * 
		 * @warning Only an approximation as there may be dead listeners
		 * who have not been cleaned up.
		 */
		template<typename Message>
		VECL_NODISCARD size_t size() const VECL_NOEXCEPT
		{
			if(!_subs.count(typeid(Message).hash_code())) return 0;

			return _subs.at(typeid(Message).hash_code()).size();
		}

		/**
		 * @return True if there are no subscriptions to Message.
		 * 
		 * @warning Only an approximation as there may be dead listeners
		 * who have not been cleaned up.
		 */
		template<typename Message>
		VECL_NODISCARD bool empty() const VECL_NOEXCEPT
		{
			return !size();
		}

		/**
		* @note MODIFIERS
		*/
		/**
		 * @brief Subscribes a function to a message type, and returns
		 * a subscription token.
		 * 
		 * @tparam Message Type of message (should inherit from Base)
		 * @tparam Func Value type of initalizer list.
		 * @tparam Args Variadic argument list
		 *
		 * @param func Function object
		 * @param args Function arguments that are binded to the function
		 * object
		 * 
		 * @return Subscription token(shared_ptr). When the token goes out
		 * of scope, the subscription is revoked.
		 */
		template<typename Message, typename Func, typename... Args>
		VECL_NODISCARD auto subscribe(Func&& func, Args&&... args)
		{
			static_assert(
				std::is_base_of_v<Base, Message>,
				"Must inherit from Base");

			return _subs[typeid(Message).hash_code()].listen(
				std::bind(func, args..., std::placeholders::_1));

		}

		/**
		 * @brief Publishes a message to all subscribers. 
		 *
		 * @tparam Message Type of message (should inherit from Base)
		 * @tparam Args Variadic argument list
		 *
		 * @param args Variadic arguments to be passed to Message constructor
		 */
		template<typename Message, typename... Args>
		void publish(Args&&... args)
		{
			static_assert(
				std::is_base_of_v<Base, Message>,
				"Must inherit from Base");

			Message msg{ std::forward<Args>(args)... };

			_subs[typeid(Message).hash_code()].trigger(msg);
		}


		/**
		 * @brief Schedules a message for a later blast.
		 *
		 * @tparam Message Type of message (should inherit from Base)
		 * @tparam Args Variadic argument list
		 *
		 * @param args Variadic arguments to be passed to Message constructor
		 */
		template<typename Message, typename... Args>
		void schedule(Args&&... args)
		{
			static_assert(
				std::is_base_of_v<Base, Message>,
				"Must inherit from Base");
			
			_queue.emplace_back(
			std::pair<uint64_t, std::shared_ptr<Base>>(
				typeid(Message).hash_code(), 
				std::make_shared<Message>(std::forward<Args>(args)...)));
		}

		/**
		 * @brief Blasts all scheduled messages.
		 *
		 */
		void blast()
		{
			for(auto [hash, msg] : _queue) 
				_subs[hash].trigger(*msg);
			_queue.clear();
		}

		/**
		 * @note NON-MEMBER FUNCTIONS
		 */

		/**
		 * @brief Swaps the contents of two publishers. The swap operation
		 * of two publishers with different memory_resource is undefined.
		 */
		friend void swap(publisher& lhs, publisher& rhs) VECL_NOEXCEPT
		{
			std::swap(lhs._subs, rhs._subs);
		}


	private:
		using subscribers = std::pmr::unordered_map<uint64_t, broadcast<callback>>;
		using message_queue = std::pmr::vector<std::pair<uint64_t, std::shared_ptr<Base>>>;
		
		subscribers _subs;
		message_queue _queue;
	};
}

#endif
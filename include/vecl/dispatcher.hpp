#ifndef VECL_DISPATCHER_H
#define VECL_DISPATCHER_H

#include "config/config.h"

#include <type_traits>
#include <functional>
#include <unordered_map>

namespace vecl
{
	template<typename Callable>
	class dispatcher
	{

	public:

		using callback = Callable;
		using token = std::shared_ptr<callback>;
		using allocator_type = std::pmr::polymorphic_allocator<std::byte>;

		explicit dispatcher(
			allocator_type mr = std::pmr::get_default_resource()
		) : _msg_subs(mr)
		{
		}

		dispatcher(const dispatcher&) = delete;
		dispatcher(dispatcher&&) = default;
		dispatcher& operator=(const dispatcher&) = delete;
		dispatcher& operator=(dispatcher&&) = default;


		template<typename Message, typename Func, typename... Args>
		VECL_NODISCARD token subscribe(Func&& func, Args&&... args)
		{
			static_assert(
				std::is_base_of_v<Base, Message>,
				"Must inherit from Base");

			token handle = std::make_shared<callback>(
				std::bind(func, args...,
					std::placeholders::_1));

			_msg_subs[typeid(Message).hash_code()].push_back(handle);

			return handle;
		}

		template<typename Message, typename... Args>
		void dispatch(Args&&... args)
		{
			Message new_msg{ std::forward<Args>(args)... };

			auto& subs = _msg_subs[typeid(Message).hash_code()];

			for (auto weak : subs)
			{
				if (auto callable = weak.lock())
					(*callable)(new_msg);
			}

			subs.erase(
				std::remove_if(
					subs.begin(), subs.end(),
					[](auto weak) { return weak.expired(); }
			), subs.end());

		}


	private:
		using weak = std::weak_ptr<callback>;
		using subscribers = std::pmr::unordered_map<uint64_t, std::pmr::vector<weak>>;

		subscribers _msg_subs;
		token _sentinel = 0;
	};
}

#endif
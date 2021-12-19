#ifndef VECL_PIPE_BASE_HPP
#define VECL_PIPE_BASE_HPP

#include "../type_traits.hpp"
#include <algorithm>

namespace vecl::pipe
{
	struct pipe_base
	{

	};

	template<typename R, typename P>
	class pipe_transform : public pipe_base
	{
	public:
		pipe_transform(std::function<T(T)>&& f) : func(f)
		{

		}

		template<typename Cont>
		auto operator<<(Cont&& c)
		{
			Cont r(c);
			return r;
		}

		std::function<T(T)> func;
	};

	template<typename Func>
	auto transform(Func f)
	{
		return pipe_transform<return_type_t<Func>>(f);
	}
}

#endif
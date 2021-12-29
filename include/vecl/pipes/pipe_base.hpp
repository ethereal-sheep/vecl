#ifndef VECL_PIPE_BASE_H
#define VECL_PIPE_BASE_H

#include "../config/config.h"

#include <functional>
#include "../type_traits.hpp"
#include "../concepts.hpp"

namespace vecl::pipes
{
	/**
	 * @brief Identity transform function to be used for function composition.
	 */
	template<typename R, typename P>
	inline static const std::function<R(P)> form_i = [](P p) constexpr -> R { return static_cast<R>(p); };

	/**
	 * @brief Identity skip function to be used for function composition.
	 */
	template<typename P>
	inline static const std::function<bool(P)> skip_i = [](P) constexpr -> bool { return false; };

	/**
	 * @brief The basic_pipe is a functor in the category theory sense. It maps objects between
	 * categories. And just like functors, we can compose them. 
	 *
	 * We do not touch the pipes with our hands; we make use of helper functions to do so.
	 * 
	 * @tparam R Return type of the pipe
	 * @tparam P Param type of the pipe
	 */
	template<typename R, typename P>
	class basic_pipe
	{
		/**
		 * @brief Default Constructor.
		 *
		 * @param mr Pointer to a pmr resource. Default gets the default
		 * global pmr resource via get_default_resource().
		 */
		template<typename C, typename V>
		requires back_insertable<C, V>
			constexpr auto insert_into(C& c, V&& v)
		{
			c.push_back(std::forward<V>(v));
		}

		/**
		 * @brief Default Constructor.
		 *
		 * @param mr Pointer to a pmr resource. Default gets the default
		 * global pmr resource via get_default_resource().
		 */
		template<typename C, typename V>
		requires insertable<C, V> && !back_insertable<C, V>
			constexpr auto insert_into(C& c, V&& v)
		{
			c.insert(std::forward<V>(v));
		}

	public:
		/**
		 * @brief Default Copy Constructor. Uses same memory resource as
		 * other.
		 */
		constexpr basic_pipe(
			std::function<R(P)> f = form_i<R, P>,
			std::function<bool(P)> s = skip_i<P>
		) : _form(f), _skip(s)
		{

		}

		/**
		 * @brief Default Copy Constructor. Uses same memory resource as
		 * other.
		 */
		using return_type = R;
		using arg_type = P;


		/**
		 * @brief Allows a container of elements to flow through the pipe.
		 */
		template<typename Cont>
		constexpr auto operator<<(const Cont& c) -> rebind_t<Cont, R>
		{
			rebind_t<Cont, R> r;
			for (auto it = c.begin(); it != c.end(); ++it)
				if (!_skip(*it))
					insert_into(r, _form(*it));
			return r;
		}

		/**
		 * @brief Monoid for compositing pipes. The return type of the compositing
		 * pipe must match the param type of the receiving pipe.
		 */
		template <typename X>
		constexpr auto operator+(basic_pipe<P, X>& rhs) -> basic_pipe<R, X>
		{
			auto form = [g = this->_form, f = rhs._form](X x){ return g(f(x)); };
			auto skip = [g = this->_skip, f = rhs._skip, h = rhs._form](X x){ return f(x) || g(h(x)); };

			return basic_pipe<R, X>(form, skip);
		}

		std::function<R(P)> _form;
		std::function<bool(P)> _skip;
	};

	/**
	 * @brief Creates a pipe from a transform function. Any element passed through this pipe is
	 * transformed by the function before being passed on.
	 */
	template<typename Func>
	constexpr auto transform(Func f) -> basic_pipe<return_type_t<Func>, type_list_element_t<0, argument_type_t<Func>>>
	{
		static_assert(!std::is_same_v<return_type_t<Func>, void>, "Func cannot have return type void");
		static_assert(argument_type_t<Func>::size == 1, "Func must have exactly one argument");

		using ret = return_type_t<Func>;
		using par = type_list_element_t<0, argument_type_t<Func>>;

		return basic_pipe<ret, par>(f);
	}

	/**
	 * @brief Creates a pipe from a filter function. Any element passed through this pipe is
	 * filtered by the function. If it filter suggests that the element should be skipped,
	 * the element is not passed through.
	 */
	template<typename Func>
	constexpr auto filter(Func f) -> basic_pipe<type_list_element_t<0, argument_type_t<Func>>, type_list_element_t<0, argument_type_t<Func>>>
	{
		static_assert(std::is_same_v<return_type_t<Func>, bool>, "Func must return bool");
		static_assert(argument_type_t<Func>::size == 1, "Func must have exactly one argument");

		using par = type_list_element_t<0, argument_type_t<Func>>;
		using ret = par;

		return basic_pipe<ret, par>(form_i<ret, par>, f);
	}

	/**
	 * @brief Creates a pipe from a value. All elements that passes through the pipe are replaced 
	 * with a new value.
	 */
	template<typename T>
	constexpr auto replace(const T& new_value) -> basic_pipe<T, T>
	{
		return basic_pipe<T, T>([v = new_value](T) { return v; });
	}

	/**
	 * @brief Creates a pipe from a value and a predicate function. All elements that passes 
	 * through the pipe are replaced with a new value if it passes the predicate.
	 */
	template<typename Func, typename T>
	constexpr auto replace_if(Func f, const T& new_value) -> basic_pipe<T, type_list_element_t<0, argument_type_t<Func>>>
	{
		static_assert(std::is_same_v<return_type_t<Func>, bool>, "Func must return bool");
		static_assert(argument_type_t<Func>::size == 1, "Func must have exactly one argument");

		using par = type_list_element_t<0, argument_type_t<Func>>;
		using ret = T;

		auto rplc = [f, v = new_value](par p) -> T {
			if (f(p))
				return v;
			return p;
		};

		return basic_pipe<ret, par>(rplc);
	}
}

#endif
#ifndef VECL_TYPE_TRAITS_HPP
#define VECL_TYPE_TRAITS_HPP

#include <type_traits>

namespace vecl
{
	template<typename... T>
	struct type_list {
		static constexpr auto size = sizeof...(T);
	};

	template<std::size_t I, class T >
	struct type_list_element;

	// recursive case
	template<std::size_t I, class Head, class... Tail >
	struct type_list_element<I, type_list<Head, Tail...>>
		: type_list_element<I - 1, type_list<Tail...>> { };

	// base case
	template<class Head, class... Tail >
	struct type_list_element<0, type_list<Head, Tail...>> {
		using type = Head;
	};

	template <std::size_t I, class T>
	using type_list_element_t = typename type_list_element<I, T>::type;


	template <typename F>
	struct return_type_impl;

	template <typename R, typename... Args>
	struct return_type_impl<R(Args...)> { using type = R; };

	template <typename R, typename... Args>
	struct return_type_impl<R(Args..., ...)> { using type = R; };

	template <typename R, typename... Args>
	struct return_type_impl<R(*)(Args...)> { using type = R; };

	template <typename R, typename... Args>
	struct return_type_impl<R(*)(Args..., ...)> { using type = R; };

	template <typename R, typename... Args>
	struct return_type_impl<R(&)(Args...)> { using type = R; };

	template <typename R, typename... Args>
	struct return_type_impl<R(&)(Args..., ...)> { using type = R; };

	template <typename R, typename C, typename... Args>
	struct return_type_impl<R(C::*)(Args...)> { using type = R; };

	template <typename R, typename C, typename... Args>
	struct return_type_impl<R(C::*)(Args..., ...)> { using type = R; };

	template <typename R, typename C, typename... Args>
	struct return_type_impl<R(C::*)(Args...)&> { using type = R; };

	template <typename R, typename C, typename... Args>
	struct return_type_impl<R(C::*)(Args..., ...)&> { using type = R; };

	template <typename R, typename C, typename... Args>
	struct return_type_impl<R(C::*)(Args...)&&> { using type = R; };

	template <typename R, typename C, typename... Args>
	struct return_type_impl<R(C::*)(Args..., ...)&&> { using type = R; };

	template <typename R, typename C, typename... Args>
	struct return_type_impl<R(C::*)(Args...) const> { using type = R; };

	template <typename R, typename C, typename... Args>
	struct return_type_impl<R(C::*)(Args..., ...) const> { using type = R; };

	template <typename R, typename C, typename... Args>
	struct return_type_impl<R(C::*)(Args...) const&> { using type = R; };

	template <typename R, typename C, typename... Args>
	struct return_type_impl<R(C::*)(Args..., ...) const&> { using type = R; };

	template <typename R, typename C, typename... Args>
	struct return_type_impl<R(C::*)(Args...) const&&> { using type = R; };

	template <typename R, typename C, typename... Args>
	struct return_type_impl<R(C::*)(Args..., ...) const&&> { using type = R; };

	template <typename R, typename C, typename... Args>
	struct return_type_impl<R(C::*)(Args...) volatile> { using type = R; };

	template <typename R, typename C, typename... Args>
	struct return_type_impl<R(C::*)(Args..., ...) volatile> { using type = R; };

	template <typename R, typename C, typename... Args>
	struct return_type_impl<R(C::*)(Args...) volatile&> { using type = R; };

	template <typename R, typename C, typename... Args>
	struct return_type_impl<R(C::*)(Args..., ...) volatile&> { using type = R; };

	template <typename R, typename C, typename... Args>
	struct return_type_impl<R(C::*)(Args...) volatile&&> { using type = R; };

	template <typename R, typename C, typename... Args>
	struct return_type_impl<R(C::*)(Args..., ...) volatile&&> { using type = R; };

	template <typename R, typename C, typename... Args>
	struct return_type_impl<R(C::*)(Args...) const volatile> { using type = R; };

	template <typename R, typename C, typename... Args>
	struct return_type_impl<R(C::*)(Args..., ...) const volatile> { using type = R; };

	template <typename R, typename C, typename... Args>
	struct return_type_impl<R(C::*)(Args...) const volatile&> { using type = R; };

	template <typename R, typename C, typename... Args>
	struct return_type_impl<R(C::*)(Args..., ...) const volatile&> { using type = R; };

	template <typename R, typename C, typename... Args>
	struct return_type_impl<R(C::*)(Args...) const volatile&&> { using type = R; };

	template <typename R, typename C, typename... Args>
	struct return_type_impl<R(C::*)(Args..., ...) const volatile&&> { using type = R; };

	template <typename T, typename = void>
	struct return_type
		: return_type_impl<T> {};

	template <typename T>
	struct return_type<T, decltype(void(&T::operator()))>
		: return_type_impl<decltype(&T::operator())> {};

	template <typename T>
	using return_type_t = typename return_type<T>::type;

	template <typename F>
	struct argument_type_impl;

	template <typename R, typename... Args>
	struct argument_type_impl<R(Args...)> { using type = type_list<Args...>; };

	template <typename R, typename... Args>
	struct argument_type_impl<R(Args..., ...)> { using type = type_list<Args...>; };

	template <typename R, typename... Args>
	struct argument_type_impl<R(*)(Args...)> { using type = type_list<Args...>; };

	template <typename R, typename... Args>
	struct argument_type_impl<R(*)(Args..., ...)> { using type = type_list<Args...>; };

	template <typename R, typename... Args>
	struct argument_type_impl<R(&)(Args...)> { using type = type_list<Args...>; };

	template <typename R, typename... Args>
	struct argument_type_impl<R(&)(Args..., ...)> { using type = type_list<Args...>; };

	template <typename R, typename C, typename... Args>
	struct argument_type_impl<R(C::*)(Args...)> { using type = type_list<Args...>; };

	template <typename R, typename C, typename... Args>
	struct argument_type_impl<R(C::*)(Args..., ...)> { using type = type_list<Args...>; };

	template <typename R, typename C, typename... Args>
	struct argument_type_impl<R(C::*)(Args...)&> { using type = type_list<Args...>; };

	template <typename R, typename C, typename... Args>
	struct argument_type_impl<R(C::*)(Args..., ...)&> { using type = type_list<Args...>; };

	template <typename R, typename C, typename... Args>
	struct argument_type_impl<R(C::*)(Args...)&&> { using type = type_list<Args...>; };

	template <typename R, typename C, typename... Args>
	struct argument_type_impl<R(C::*)(Args..., ...)&&> { using type = type_list<Args...>; };

	template <typename R, typename C, typename... Args>
	struct argument_type_impl<R(C::*)(Args...) const> { using type = type_list<Args...>; };

	template <typename R, typename C, typename... Args>
	struct argument_type_impl<R(C::*)(Args..., ...) const> { using type = type_list<Args...>; };

	template <typename R, typename C, typename... Args>
	struct argument_type_impl<R(C::*)(Args...) const&> { using type = type_list<Args...>; };

	template <typename R, typename C, typename... Args>
	struct argument_type_impl<R(C::*)(Args..., ...) const&> { using type = type_list<Args...>; };

	template <typename R, typename C, typename... Args>
	struct argument_type_impl<R(C::*)(Args...) const&&> { using type = type_list<Args...>; };

	template <typename R, typename C, typename... Args>
	struct argument_type_impl<R(C::*)(Args..., ...) const&&> { using type = type_list<Args...>; };

	template <typename R, typename C, typename... Args>
	struct argument_type_impl<R(C::*)(Args...) volatile> { using type = type_list<Args...>; };

	template <typename R, typename C, typename... Args>
	struct argument_type_impl<R(C::*)(Args..., ...) volatile> { using type = type_list<Args...>; };

	template <typename R, typename C, typename... Args>
	struct argument_type_impl<R(C::*)(Args...) volatile&> { using type = type_list<Args...>; };

	template <typename R, typename C, typename... Args>
	struct argument_type_impl<R(C::*)(Args..., ...) volatile&> { using type = type_list<Args...>; };

	template <typename R, typename C, typename... Args>
	struct argument_type_impl<R(C::*)(Args...) volatile&&> { using type = type_list<Args...>; };

	template <typename R, typename C, typename... Args>
	struct argument_type_impl<R(C::*)(Args..., ...) volatile&&> { using type = type_list<Args...>; };

	template <typename R, typename C, typename... Args>
	struct argument_type_impl<R(C::*)(Args...) const volatile> { using type = type_list<Args...>; };

	template <typename R, typename C, typename... Args>
	struct argument_type_impl<R(C::*)(Args..., ...) const volatile> { using type = type_list<Args...>; };

	template <typename R, typename C, typename... Args>
	struct argument_type_impl<R(C::*)(Args...) const volatile&> { using type = type_list<Args...>; };

	template <typename R, typename C, typename... Args>
	struct argument_type_impl<R(C::*)(Args..., ...) const volatile&> { using type = type_list<Args...>; };

	template <typename R, typename C, typename... Args>
	struct argument_type_impl<R(C::*)(Args...) const volatile&&> { using type = type_list<Args...>; };

	template <typename R, typename C, typename... Args>
	struct argument_type_impl<R(C::*)(Args..., ...) const volatile&&> { using type = type_list<Args...>; };

	template <typename T, typename = void>
	struct argument_type
		: argument_type_impl<T> {};

	template <typename T>
	struct argument_type<T, decltype(void(&T::operator()))>
		: argument_type_impl<decltype(&T::operator())> {};

	template <typename T>
	using argument_type_t = typename argument_type<T>::type;

	template<typename C, typename R>
	struct rebind;

	template<typename V, typename... Args, template<typename ...> typename C, typename R>
	struct rebind<C<V, Args...>, R> { using type = C<R, typename rebind<Args, R>::type...>; };

	template <typename Cont, typename R>
	using rebind_t = typename rebind<Cont, R>::type;

}


#endif
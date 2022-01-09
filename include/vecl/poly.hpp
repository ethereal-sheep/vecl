#pragma once

#include <any>
#include <tuple>
#include <iostream>
#include <typeinfo>
#include <vecl/type_traits.hpp>

namespace vecl
{
	template<auto... T>
	struct value_list {
		using type = value_list;

		static constexpr auto size = sizeof...(T);
	};

	template<std::size_t, typename>
	struct value_list_element;

	// recursive case
	template<std::size_t I, auto Head, auto... Tail >
	struct value_list_element<I, value_list<Head, Tail...>>
		: value_list_element<I - 1, value_list<Tail...>> { };

	// base case
	template<auto Head, auto... Tail >
	struct value_list_element<0, value_list<Head, Tail...>> {
		static constexpr auto value = Head;
	};

	template <std::size_t I, typename List>
	constexpr auto value_list_element_v = value_list_element<I, List>::value;
}

namespace vecl
{
	template <auto... Values>
	using poly_members = value_list<Values...>;

	struct poly_inspector
	{
		template<class Type>
		operator Type&& () const 
		{
			return static_cast<Type&&>(*this);
		}

		template <auto Member, typename... Args>
		poly_inspector invoke(Args&& ...) const
		{ 
			return *this; 
		}

		template <auto Member, typename... Args>
		poly_inspector invoke(Args&& ...)
		{ 
			return *this;  
		}
	};

	template <typename Concept>
	class poly;

	template <typename Concept>
	class vtable
	{
	private:
		friend poly<Concept>;

		using Inspector = Concept:: template Interface<poly_inspector>;
		using Members = Concept:: template Members<Inspector>;

		template <typename Ret, typename Cls, typename... Args>
		static constexpr auto 
		build_vtable_entry(Ret(Cls::*)(Args...))
			-> Ret(*)(std::any&, Args...);

		template <typename Ret, typename Cls, typename... Args>
		static constexpr auto 
		build_vtable_entry(Ret(Cls::*)(Args...) const)
			-> Ret(*)(const std::any&, Args...);

		template <typename Concept, std::size_t... Index>
		static constexpr decltype(auto) build_vtable(std::index_sequence<Index...>)
		{
			return std::make_tuple(
				build_vtable_entry(vecl::value_list_element_v<Index, Members>)...);
		}

		template <typename Concept>
		static constexpr decltype(auto) build_vtable()
		{
			return build_vtable<Concept>(std::make_index_sequence<Members::size>{});
		}

		template <typename Type, auto Candidate, typename Ret, typename Any, typename... Args>
		static constexpr decltype(auto) get_vtable_entry()
		{
			return +[](Any& any, Args... args)
			{
				return std::invoke(Candidate, std::any_cast<Type>(any), std::forward<Args>(args)...);
			};
		}

		template <typename Type, auto Candidate, typename Ret, typename Any, typename... Args>
		static constexpr auto get_vtable_entry(Ret(*)(Any&, Args...))
		{
			return get_vtable_entry<Type, Candidate, Ret, Any, Args...>();
		}

		template <typename Type, std::size_t... Index>
		static constexpr decltype(auto) get_vtable(std::index_sequence<Index...>)
		{
			using Table = vtable<Concept>::type;

			return std::make_tuple(
				get_vtable_entry<Type, vecl::value_list_element_v<Index, Members>>(
					std::get<Index>(Table{}))...);
		}

		template <typename Type>
		static constexpr decltype(auto) get_ptr()
		{
			using Members = Concept:: template Members<Type>;
			static constexpr auto vtable = get_vtable<Type>(std::make_index_sequence<Members::size>{});

			return &vtable;
		}
	
		using type = decltype(build_vtable<Concept>());

	public:
		vtable() = delete;

	};

	template <typename Poly>
	class poly_base
	{
	private:
		template<auto Member, typename Poly, typename... Args>
		friend decltype(auto) poly_call(Poly&&, Args&& ...);

		template <auto Member, typename... Args>
		[[nodiscard]] decltype(auto) invoke(const poly_base& self, Args&& ...args) const
		{
			return static_cast<const Poly&>(self).invoke<Member>(std::forward<Args>(args)...);
		}

		template <auto Member, typename... Args>
		[[nodiscard]] decltype(auto) invoke(poly_base& self, Args&& ...args)
		{
			return static_cast<Poly&>(self).invoke<Member>(std::forward<Args>(args)...);
		}
	};
	
	template <typename Concept>
	class poly : public Concept:: template Interface<poly_base<poly<Concept>>>
	{
		using VTable = vtable<Concept>::type;

		std::any	  _underlying;
		const VTable* _vtable_ptr;

	private:
		friend poly_base<poly<Concept>>;

		template <auto Member, typename... Args>
		[[nodiscard]] decltype(auto) invoke(Args&& ...args)
		{
			return std::get<Member>(*_vtable_ptr)(_underlying, std::forward<Args>(args)...);
		}

		template <auto Member, typename... Args>
		[[nodiscard]] decltype(auto) invoke(Args&& ...args) const
		{
			return std::get<Member>(*_vtable_ptr)(_underlying, std::forward<Args>(args)...);
		}

	public:
		template <typename Any>
		poly(Any&& any)
			: _underlying{std::forward<Any>(any)}
			, _vtable_ptr{ vtable<Concept>::template get_ptr<Any>() }
		{
		}
	};

	template<auto Member, typename Poly, typename... Args>
	decltype(auto) poly_call(Poly&& self, Args&& ...args)
	{
		return std::forward<Poly>(self). 
				template invoke<Member>(self, std::forward<Args>(args)...);
	}
}




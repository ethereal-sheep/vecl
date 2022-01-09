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
	template <auto... Ts>
	using PolyMembers = value_list<Ts...>;

	struct PolyInspector
	{
		template<class Type>
		operator Type&& () const 
		{
			return static_cast<Type&&>(*this);
		}

		template <auto Member, typename... Args>
		PolyInspector invoke(Args&& ...) const 
		{ 
			return *this; 
		}

		template <auto Member, typename... Args>
		PolyInspector invoke(Args&& ...) 
		{ 
			return *this;  
		}
	};

	template <typename Ret, typename Cls, typename... Args>
	auto BuildTableEntry(Ret(Cls::*)(Args...))->Ret(*)(std::any&, Args...);

	template <typename Ret, typename Cls, typename... Args>
	auto BuildTableEntry(Ret(Cls::*)(Args...) const)->Ret(*)(const std::any&, Args...);

	template <typename Concept, std::size_t... Index>
	decltype(auto) BuildTable(std::index_sequence<Index...>)
	{
		using Inspector = Concept:: template Interface<PolyInspector>;
		using Members = Concept:: template Members<Inspector>;
		return std::make_tuple(BuildTableEntry(vecl::value_list_element_v<Index, Members>)...);
	}

	template <typename Concept>
	decltype(auto) BuildTable()
	{
		using Inspector = Concept:: template Interface<PolyInspector>;
		using Members = Concept:: template Members<Inspector>;
		return BuildTable<Concept>(std::make_index_sequence<Members::size>{});
	}

	template <typename Type, auto Candidate, typename Ret, typename Any, typename... Args>
	decltype(auto) BuildTableEntry()
	{
		return +[](Any& any, Args... args)
		{
			return std::invoke(Candidate, std::any_cast<Type>(any), std::forward<Args>(args)...);
		};
	}

	template <typename Type, auto Candidate, typename Ret, typename Any, typename... Args>
	auto BuildTableEntry(Ret(*)(Any&, Args...))
	{
		return BuildTableEntry<Type, Candidate, Ret, Any, Args...>();
	}

	template <typename Concept, typename Type, std::size_t... Index>
	constexpr decltype(auto) FillTable(std::index_sequence<Index...>)
	{
		using Table = decltype(BuildTable<Concept>());
		using Members = Concept:: template Members<Type>;

		static constexpr Table table;
		return std::make_tuple(
			BuildTableEntry<Type, vecl::value_list_element_v<Index, Members>>(
				std::get<Index>(table))...);
	}

	template <typename Concept, typename Type>
	decltype(auto) FillTable()
	{
		using Members = Concept:: template Members<Type>;
		static auto table = FillTable<Concept, Type>(std::make_index_sequence<Members::size>{});

		return &table;
	}

	template <typename Poly>
	class PolyBase
	{
	private:
		template<auto Member, typename Poly, typename... Args>
		friend decltype(auto) poly_call(Poly&& self, Args&& ...args);

		template <auto Member, typename... Args>
		[[nodiscard]] decltype(auto) invoke(const PolyBase& self, Args&& ...args) const
		{
			return static_cast<const Poly&>(self).invoke<Member>(std::forward<Args>(args)...);
		}

		template <auto Member, typename... Args>
		[[nodiscard]] decltype(auto) invoke(PolyBase& self, Args&& ...args)
		{
			return static_cast<Poly&>(self).invoke<Member>(std::forward<Args>(args)...);
		}
	};
	
	template <typename Concept>
	class Poly : public Concept:: template Interface<PolyBase<Poly<Concept>>>
	{
		using VTable = decltype(BuildTable<Concept>());

		std::any	  _underlying;
		const VTable* _vtable;

	public:
		template <typename Any>
		Poly(Any&& any)
			: _underlying{std::forward<Any>(any)}
			, _vtable { FillTable<Concept, Any>() }
		{
		}

	private:
		friend PolyBase<Poly<Concept>>;

		template <auto Member, typename... Args>
		[[nodiscard]] decltype(auto) invoke(Args&& ...args)
		{
			return std::get<Member>(*_vtable)(_underlying, std::forward<Args>(args)...);
		}

		template <auto Member, typename... Args>
		[[nodiscard]] decltype(auto) invoke(Args&& ...args) const
		{
			return std::get<Member>(*_vtable)(_underlying, std::forward<Args>(args)...);
		}
	};

	template<auto Member, typename Poly, typename... Args>
	decltype(auto) poly_call(Poly&& self, Args&& ...args)
	{
		return std::forward<Poly>(self). 
				template invoke<Member>(self, std::forward<Args>(args)...);
	}
}




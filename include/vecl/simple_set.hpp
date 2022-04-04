#ifndef VECL_SIMPLE_SET_H
#define VECL_SIMPLE_SET_H


#ifndef VECL_DEFAULT_SET_SIZE
#define VECL_DEFAULT_SET_SIZE 8
#endif

#include "config/config.h"

#include "simple_buffer.hpp"
#include <functional>
#include <algorithm> // upper bound
#include <optional>

namespace vecl
{

	constexpr size_t hashtable_primes[] = 
		{ 53, 97, 193, 389, 769, 1543, 3079, 
		6151, 12289, 24593, 49157, 98317, 
		196613, 393241, 786433, 1572869, 
		3145739, 6291469, 12582917,25165843, 
		50331653, 100663319, 201326611, 
		402653189, 805306457, 1610612741 };

	struct probe_seq
	{
		probe_seq(size_t hash, size_t size)
			: _hash{ hash }, _size{ size }, _offset{ 0 }, _index{ _hash % _size } {}

		void next()
		{
			++_offset;
			_index = _hash + (_offset * _offset);
			_index %= _size;
		}

		auto index() const { return _index; }

	private:
		size_t _hash;
		size_t _size;
		size_t _offset;
		size_t _index;
	};

	
	// use open addressing
	// and quadratic probing
	template<typename T, typename Hash = std::hash<T>, typename Equal = std::equal_to<T>>
	class simple_set
	{
		struct simple_set_node
		{
			using value_type = T;

			template<typename... TArgs>
			simple_set_node(TArgs&&... args)
			{
				construct(std::forward<TArgs>(args)...);
			}

			~simple_set_node()
			{
				destroy();
			}

			void destroy()
			{
				if constexpr (!std::is_trivially_destructible_v<T>)
					if(_alive)
						std::destroy(data());

				_alive = false;
			}

			template<typename... TArgs>
			void construct(TArgs&&... args)
			{
				if (!_alive)
					std::construct_at(data(), std::forward<TArgs>(args)...);

				_alive = true;
			}

			VECL_NODISCARD bool alive() const { return _alive; }

			VECL_NODISCARD value_type& get()
			{
				if (!_alive)
					throw std::bad_optional_access();

				return *data();
			}

			VECL_NODISCARD const value_type& get() const
			{
				if (!_alive)
					throw std::bad_optional_access();

				return *data();
			}

			VECL_NODISCARD value_type* get_if()
			{
				if (!_alive)
					return nullptr;

				return data();
			}

			VECL_NODISCARD const value_type* get_if() const
			{
				if (!_alive)
					return nullptr;

				return data();
			}


		private:

			static constexpr bool _is_sufficiently_trivial =
				std::is_trivially_default_constructible_v<T> && 
				std::is_trivially_destructible_v<T>;

			VECL_NODISCARD T* data()
			{
				if constexpr (_is_sufficiently_trivial)
					return &_value;
				else
					return reinterpret_cast<T*>(std::addressof(_value));
			}

			VECL_NODISCARD T* data() const
			{
				if constexpr (_is_sufficiently_trivial)
					return &_value;
				else
					return reinterpret_cast<const T*>(std::addressof(_value));
			}

			using storage_type = std::conditional_t<
				_is_sufficiently_trivial, value_type, char[sizeof(T)]>;

			alignas(T) storage_type _value;
			bool _alive = false;
		};

	public:
		using array_type = simple_buffer<T*>;
		using array_type_reference = array_type&;
		using const_array_type_reference = const array_type&;
		using key_type = T;
		using size_type = size_t;

	private:
		static void _insert_into(array_type_reference arr, const key_type& key)
		{
			probe_seq seq(Hash{}(key), arr.capacity());
			while (arr[seq.index()] != nullptr) seq.next();

			arr[seq.index()] = new key_type(key);
		}

		static void _insert_into(array_type_reference arr, key_type* key)
		{
			probe_seq seq(Hash{}(*key), arr.capacity());
			while (arr[seq.index()] != nullptr) seq.next();

			arr[seq.index()] = key;
		}

		static bool _contained_in(const_array_type_reference arr, const key_type& key)
		{
			probe_seq seq(Hash{}(key), arr.capacity());
			while (arr[seq.index()] != nullptr)
			{
				if (Equal{}(*arr[seq.index()], key))
					return true;

				seq.next();
			}

			return false;
		}

		static bool _should_grow(size_t curr_size, const_array_type_reference arr)
		{
			return (1.f * curr_size / arr.capacity()) > 0.7f;
		}

		void _grow_table()
		{
			auto new_size = std::upper_bound(
				std::begin(hashtable_primes), 
				std::end(hashtable_primes),
				_arr.capacity() + 1);

			array_type new_arr(*new_size, nullptr);
			for (auto& ptr : _arr)
				if (ptr) _insert_into(new_arr, ptr);
			_arr = std::move(new_arr);
		}

	public:
		simple_set() : _size{ 0 }, _arr(8, nullptr) {}

		explicit simple_set(size_type size_hint)
			: _size{ 0 }, _arr(std::max((size_t)VECL_DEFAULT_SET_SIZE, size_hint), nullptr) {}
		
		template<typename It>
		simple_set(
			It first, It last, 
			size_type size_hint = VECL_DEFAULT_SET_SIZE
		) : _size{ 0 }, _arr(std::max((size_t)VECL_DEFAULT_SET_SIZE, size_hint), nullptr)
		{
			for (; first != last; ++first)
				insert(static_cast<key_type>(*first));
		}

		template <typename TArg>
		simple_set(
			std::initializer_list<TArg> il, 
			size_type size_hint = (size_t)VECL_DEFAULT_SET_SIZE
		) : simple_set(il.begin(), il.end(), size_hint)	{}



		~simple_set()
		{
			clear();
		}


		bool insert(const key_type& key)
		{
			if (contains(key))
				return false;

			if (_should_grow(_size+1, _arr))
				_grow_table();

			_insert_into(_arr, key);
			++_size;

			return true;
		}

		auto contains(const key_type& key) const
		{
			return _contained_in(_arr, key);
		}

		size_t count(const key_type& key) const
		{
			return _contained_in(_arr, key) ? 1 : 0;
		}

		auto size() const { return _size; }
		auto capacity() const { return _arr.capacity(); }
		auto load_factor() const { return 1.f * size() / capacity(); }

		auto clear()
		{
			for (auto& ptr : _arr)
			{
				if (ptr) 
				{
					delete ptr;
					ptr = nullptr;
				}
			}
		}
		

	private:
		size_type _size;
		array_type _arr;
	};

}

#endif
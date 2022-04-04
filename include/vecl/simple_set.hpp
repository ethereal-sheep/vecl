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
		{ 11, 17, 29, 53, 97, 193, 389, 769, 1543, 3079, 
		6151, 12289, 24593, 49157, 98317, 
		196613, 393241, 786433, 1572869, 
		3145739, 6291469, 12582917,25165843, 
		50331653, 100663319, 201326611, 
		402653189, 805306457, 1610612741 };

	template<size_t Width = 1>
	struct probe_seq
	{
		probe_seq(size_t hash, size_t size)
			: _hash{ hash }, _size{ size }, _offset{ 0 }, _index{ _hash % _size } {}

		void next()
		{
			_index += Width;
			_offset = _hash + _index;
			_offset %= _size;
		}

		auto index() const { return _index; }
		auto hash() const { return _hash; }
		auto offset() const { return _offset; }

	private:
		size_t _hash;
		size_t _size;
		size_t _offset;
		size_t _index;
	};

	
	// use open addressing
	// and quadratic probing
	template<
		typename T, 
		typename Hash = std::hash<T>, 
		typename Equal = std::equal_to<T>
	>
	class simple_set
	{
		struct simple_set_node
		{
			using value_type = T;

			template<typename... TArgs>
			simple_set_node(size_t hash, TArgs&&... args)
			{
				construct(hash, std::forward<TArgs>(args)...);
			}

			~simple_set_node()
			{
				destroy();
			}

			template<typename... TArgs>
			void construct(size_t hash, TArgs&&... args)
			{
				if (!_alive)
				{
					_hash = hash;
					std::construct_at(data(), std::forward<TArgs>(args)...);
				}

				_alive = true;
			}

			void destroy()
			{
				if constexpr (!std::is_trivially_destructible_v<T>)
					if (_alive)
						std::destroy_at(data());

				_alive = false;
			}

			VECL_NODISCARD bool alive() const { return _alive; }
			
			VECL_NODISCARD auto hash() const { return _hash; }

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

			T* data()
			{
				if constexpr (_is_sufficiently_trivial)
					return &_value;
				else
					return reinterpret_cast<T*>(std::addressof(_value));
			}

			T* data() const
			{
				if constexpr (_is_sufficiently_trivial)
					return &_value;
				else
					return reinterpret_cast<const T*>(std::addressof(_value));
			}

			using storage_type = std::conditional_t<
				_is_sufficiently_trivial, value_type, char[sizeof(T)]>;

			size_t _hash = 0;
			alignas(T) storage_type _value;
			bool _alive = false;
		};

	public:
		using array_type = simple_buffer<simple_set_node*>;
		using array_type_reference = array_type&;
		using const_array_type_reference = const array_type&;
		using key_type = T;
		using size_type = size_t;

	private:
		template<typename... TArgs>
		static bool _emplace_into_slot(size_t hash, simple_set_node*& slot, TArgs&&... args)
		{
			if (slot == nullptr)
			{
				slot = new simple_set_node(hash, std::forward<TArgs>(args)...);
				return true;
			}
			else if (!slot->alive())
			{
				slot->construct(hash, std::forward<TArgs>(args)...);
			}

			return false;

		}

		static bool _insert_into(array_type_reference arr, const key_type& key)
		{
			probe_seq seq(Hash{}(key), arr.capacity());
			while (arr[seq.offset()] != nullptr && arr[seq.offset()]->alive()) seq.next();

			return _emplace_into_slot(seq.hash(), arr[seq.offset()], key);
		}

		static bool _try_insert_into_empty(array_type_reference arr, simple_set_node* node)
		{
			if (node == nullptr || !node->alive()) return false;

			// assume empty table has no deletions, so we dont check here
			probe_seq seq(node->hash(), arr.capacity());
			while (arr[seq.offset()] != nullptr) seq.next();

			arr[seq.offset()] = node;

			return true;
		}

		static bool _try_erase_from(array_type_reference arr, const key_type& key)
		{
			probe_seq seq(Hash{}(key), arr.capacity());
			while (arr[seq.offset()] != nullptr)
			{
				if (arr[seq.offset()]->alive() && Equal{}(arr[seq.offset()]->get(), key))
				{
					arr[seq.offset()]->destroy();
					return true;
				}

				seq.next();
			}
			return false;
		}

		static bool _contained_in(const_array_type_reference arr, const key_type& key)
		{
			probe_seq seq(Hash{}(key), arr.capacity());
			while (arr[seq.offset()] != nullptr)
			{
				if (arr[seq.offset()]->alive() && Equal{}(arr[seq.offset()]->get(), key))
					return true;

				seq.next();
			}

			return false;
		}

		auto _probe_length()
		{
			return load_factor() / (1 - load_factor());
		}

		bool _should_grow()
		{
			return load_factor() > 0.75f;
		}

		void _grow_table()
		{
			/*auto new_size = std::upper_bound(
				std::begin(hashtable_primes), 
				std::end(hashtable_primes),
				_arr.capacity() + 1);*/

			array_type new_arr(capacity() * 2, nullptr);
			_load = 0;
			for (auto& ptr : _arr)
			{
				if(_try_insert_into_empty(new_arr, ptr))
					++_load;
			}
			_arr = std::move(new_arr);
		}

	public:
		simple_set() : _arr(8, nullptr) {}

		explicit simple_set(size_type size_hint)
			: _arr(std::max((size_t)VECL_DEFAULT_SET_SIZE, size_hint), nullptr) {}
		
		template<typename It>
		simple_set(
			It first, It last, 
			size_type size_hint = VECL_DEFAULT_SET_SIZE
		) : _arr(std::max((size_t)VECL_DEFAULT_SET_SIZE, size_hint), nullptr)
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

			if (_should_grow())
				_grow_table();

			if (_insert_into(_arr, key))
				++_load;

			++_size;

			return true;
		}

		bool erase(const key_type& key)
		{
			if (_try_erase_from(_arr, key))
				return --_size, true;
			return false;
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

		auto load_factor() const { return 1.f * _load / capacity(); }

		void clear()
		{
			for (auto& ptr : _arr)
			{
				if (ptr) 
				{
					delete ptr;
					ptr = nullptr;
				}
			}

			_size = 0;
			_load = 0;
		}

		friend std::ostream& operator<<(std::ostream& os, const simple_set& set)
		{
			for (auto i = 0; i < (int)set._arr.capacity(); ++i)
			{
				if (i && i % 29 == 0)
					os << "\n";

				os << "[";

				if (set._arr[i] == nullptr)
					os << " ";
				else if(!set._arr[i]->alive())
					os << "x";
				else
					os << set._arr[i]->get();

				os << "]";
			}
			return os;
		}

	private:
		array_type _arr;
		size_type _load = 0;
		size_type _size = 0;
	};

	// use stable addressing
	// and chaining
	template<
		typename T,
		typename Hash = std::hash<T>,
		typename Equal = std::equal_to<T>
	>
		class second_simple_set
	{
		struct simple_set_node
		{
			using value_type = T;

			template<typename... TArgs>
			simple_set_node(size_t hash, TArgs&&... args)
			{
				construct(hash, std::forward<TArgs>(args)...);
			}

			~simple_set_node()
			{
				destroy();
			}

			template<typename... TArgs>
			void construct(size_t hash, TArgs&&... args)
			{
				if (!_alive)
				{
					_hash = hash;
					std::construct_at(data(), std::forward<TArgs>(args)...);
				}

				_alive = true;
			}

			void destroy()
			{
				if constexpr (!std::is_trivially_destructible_v<T>)
					if (_alive)
						std::destroy_at(data());

				_alive = false;
			}

			VECL_NODISCARD bool alive() const { return _alive; }

			VECL_NODISCARD auto hash() const { return _hash; }

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

			T* data()
			{
				if constexpr (_is_sufficiently_trivial)
					return &_value;
				else
					return reinterpret_cast<T*>(std::addressof(_value));
			}

			T* data() const
			{
				if constexpr (_is_sufficiently_trivial)
					return &_value;
				else
					return reinterpret_cast<const T*>(std::addressof(_value));
			}

			using storage_type = std::conditional_t<
				_is_sufficiently_trivial, value_type, char[sizeof(T)]>;

			size_t _hash = 0;
			alignas(T) storage_type _value;
			bool _alive = false;
		};

	public:
		using array_type = simple_buffer<simple_set_node*>;
		using array_type_reference = array_type&;
		using const_array_type_reference = const array_type&;
		using key_type = T;
		using size_type = size_t;

	private:
		template<typename... TArgs>
		static bool _emplace_into_slot(size_t hash, simple_set_node*& slot, TArgs&&... args)
		{
			if (slot == nullptr)
			{
				slot = new simple_set_node(hash, std::forward<TArgs>(args)...);
				return true;
			}
			else if (!slot->alive())
			{
				slot->construct(hash, std::forward<TArgs>(args)...);
			}

			return false;

		}

		static bool _insert_into(array_type_reference arr, const key_type& key)
		{
			probe_seq seq(Hash{}(key), arr.capacity());
			while (arr[seq.offset()] != nullptr && arr[seq.offset()]->alive()) seq.next();

			return _emplace_into_slot(seq.hash(), arr[seq.offset()], key);
		}

		static bool _try_insert_into_empty(array_type_reference arr, simple_set_node* node)
		{
			if (node == nullptr || !node->alive()) return false;

			// assume empty table has no deletions, so we dont check here
			probe_seq seq(node->hash(), arr.capacity());
			while (arr[seq.offset()] != nullptr) seq.next();

			arr[seq.offset()] = node;

			return true;
		}

		static bool _try_erase_from(array_type_reference arr, const key_type& key)
		{
			probe_seq seq(Hash{}(key), arr.capacity());
			while (arr[seq.offset()] != nullptr)
			{
				if (arr[seq.offset()]->alive() && Equal {}(arr[seq.offset()]->get(), key))
				{
					arr[seq.offset()]->destroy();
					return true;
				}

				seq.next();
			}
			return false;
		}

		static bool _contained_in(const_array_type_reference arr, const key_type& key)
		{
			probe_seq seq(Hash{}(key), arr.capacity());
			while (arr[seq.offset()] != nullptr)
			{
				if (arr[seq.offset()]->alive() && Equal {}(arr[seq.offset()]->get(), key))
					return true;

				seq.next();
			}

			return false;
		}

		auto _probe_length()
		{
			return load_factor() / (1 - load_factor());
		}

		bool _should_grow()
		{
			return load_factor() > 0.75f;
		}

		void _grow_table()
		{
			/*auto new_size = std::upper_bound(
				std::begin(hashtable_primes),
				std::end(hashtable_primes),
				_arr.capacity() + 1);*/

			array_type new_arr(capacity() * 2, nullptr);
			_load = 0;
			for (auto& ptr : _arr)
			{
				if (_try_insert_into_empty(new_arr, ptr))
					++_load;
			}
			_arr = std::move(new_arr);
		}

	public:
		second_simple_set() : _arr(8, nullptr) {}

		explicit second_simple_set(size_type size_hint)
			: _arr(std::max((size_t)VECL_DEFAULT_SET_SIZE, size_hint), nullptr) {}

		template<typename It>
		second_simple_set(
			It first, It last,
			size_type size_hint = VECL_DEFAULT_SET_SIZE
		) : _arr(std::max((size_t)VECL_DEFAULT_SET_SIZE, size_hint), nullptr)
		{
			for (; first != last; ++first)
				insert(static_cast<key_type>(*first));
		}

		template <typename TArg>
		second_simple_set(
			std::initializer_list<TArg> il,
			size_type size_hint = (size_t)VECL_DEFAULT_SET_SIZE
		) : simple_set(il.begin(), il.end(), size_hint) {}


		~second_simple_set()
		{
			clear();
		}


		bool insert(const key_type& key)
		{
			if (contains(key))
				return false;

			if (_should_grow())
				_grow_table();

			if (_insert_into(_arr, key))
				++_load;

			++_size;

			return true;
		}

		bool erase(const key_type& key)
		{
			if (_try_erase_from(_arr, key))
				return --_size, true;
			return false;
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

		auto load_factor() const { return 1.f * _load / capacity(); }

		void clear()
		{
			for (auto& ptr : _arr)
			{
				if (ptr)
				{
					delete ptr;
					ptr = nullptr;
				}
			}

			_size = 0;
			_load = 0;
		}

		friend std::ostream& operator<<(std::ostream& os, const second_simple_set& set)
		{
			for (auto i = 0; i < (int)set._arr.capacity(); ++i)
			{
				if (i && i % 29 == 0)
					os << "\n";

				os << "[";

				if (set._arr[i] == nullptr)
					os << " ";
				else if (!set._arr[i]->alive())
					os << "x";
				else
					os << set._arr[i]->get();

				os << "]";
			}
			return os;
		}

	private:
		array_type _arr;
		size_type _load = 0;
		size_type _size = 0;
	};

}

#endif
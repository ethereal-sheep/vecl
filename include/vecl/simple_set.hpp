#ifndef VECL_SIMPLE_SET_H
#define VECL_SIMPLE_SET_H


#include "config/config.h"

#include "simple_buffer.hpp"
#include <functional>
#include <algorithm> // upper bound
#include <optional>
#include <iomanip> // setw

namespace vecl
{

	// Inline ?  Stored inplace in backing array : Stored as separate node
	// StoreHash ? Store hash(better performance on rehash) : Do not store hash(better memory footprint)
	// GrowthPolicy - Prime Growth | PowerOfTwo Growth
	// ProbePolicy - Linear | Quadratic | Double
	// HeterogeneousLookup

	namespace detail
	{

		constexpr size_t hashtable_powers_of_two[] =
		{ 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096,
		8192, 16384, 32768, 65536, 131072, 262144, 524288,
		1048576, 2097152, 4194304, 8388608, 16777216,
		33554432, 67108864, 134217728, 268435456, 536870912,
		1073741824, 2147483648, 4294967296, 8589934592 };

		constexpr size_t hashtable_primes[] =
		{ 11, 17, 29, 53, 97, 193, 389, 769, 1543, 3079,
		6151, 12289, 24593, 49157, 98317,
		196613, 393241, 786433, 1572869,
		3145739, 6291469, 12582917,25165843,
		50331653, 100663319, 201326611,
		402653189, 805306457, 1610612741 };


		struct power_of_two_growth
		{
			// always start with 8
			constexpr size_t start(size_t requested) const
			{
				return *std::upper_bound(
					std::begin(hashtable_powers_of_two),
					std::end(hashtable_powers_of_two),
					requested);
			}

			constexpr size_t next(size_t current) const
			{
				return current << 1;
			}
		};

		struct prime_growth
		{
			// always start with 11
			constexpr size_t start(size_t requested) const
			{
				return *std::upper_bound(
					std::begin(hashtable_primes),
					std::end(hashtable_primes),
					requested);
			}

			constexpr size_t next(size_t current) const
			{
				return *std::upper_bound(
					std::begin(hashtable_primes),
					std::end(hashtable_primes),
					current + 1);
			}
		};

		template<typename LazyTValue, typename TValue>
		struct is_lazy_value
		{
			static constexpr bool value = std::regular_invocable<LazyTValue> &&
				std::is_same<TValue, std::decay<std::invoke_result_t<LazyTValue>>>::value;
		};

		template<typename LazyTValue, typename TValue>
		static constexpr bool is_lazy_value_v = is_lazy_value<LazyTValue, TValue>::value;

		template<typename LazyTValue, typename TValue>
		concept lazy_value = is_lazy_value_v<LazyTValue, TValue>;


		template<typename TKey, typename TValue>
		struct map_lookup_result
		{
			explicit operator bool() const { return _key != nullptr; }

			TKey& key() const { return *_key; }
			TValue& value() const { return *_value; }

		private:
			TKey* _key = nullptr;
			TValue* _value = nullptr;
		};

		template<typename TKey, typename TValue>
		struct map_insert_result
		{
			explicit operator bool() const { return _inserted; }

			TKey& key() const { return *_key; }
			TValue& value() const { return *_value; }

		private:
			TKey* _key = nullptr;
			TValue* _value = nullptr;
			bool _inserted;
		};

		template<typename TKey, typename TValue>
		struct map_remove_result
		{
			explicit operator bool() const { return _key; }

			TKey& key() { return *_key; }
			TValue& value() { return *_value; }

			TKey take_key() const { return *std::move(_key); }
			TValue take_value() const { return *std::move(_value); }
		private:
			std::optional<TKey> _key;
			std::optional<TValue> _value;
		};


		template<
			typename TKey,
			typename TValue,
			typename Hash = std::hash<TKey>,
			typename Equal = std::equal_to<TKey>
		>
			struct map_interface
		{
			using size_type = size_t;
			using hash_type = Hash;
			using equal_type = Equal;
			using lookup_result_type = map_lookup_result<TKey, TValue>;
			using insert_result_type = map_insert_result<TKey, TValue>;
			using remove_result_type = map_remove_result<TKey, TValue>;

			template<typename TOther>
			auto lookup(const TOther&) const->lookup_result_type;

			template<typename TOther>
			auto insert(const TOther&, TValue)->insert_result_type;

			template<typename TOther, lazy_value LazyTValue>
			auto insert(const TOther&, LazyTValue&)->insert_result_type;

			template<typename TOther>
			auto update(const TOther&, TValue)->insert_result_type;

			template<typename TOther, lazy_value LazyTValue>
			auto update(const TOther&, LazyTValue&)->insert_result_type;

			template<typename TOther>
			auto remove(const TOther&)->remove_result_type;

			auto operator[](const TKey&)->TValue&;

			auto operator[](const TKey&) const->const TValue&;

			auto clear() -> void;

			template<typename TOther>
			auto contains(const TOther&) const -> bool;

			auto size() const->size_type;
		};


		template<size_t Width, bool PowerOfTwo>
		struct linear_probe_seq
		{
			linear_probe_seq(size_t hash, size_t size)
				: _hash{ hash }, _size{ size }, _offset{ _hash % _size }, _index{ 0 } {}

			void next()
			{
				_index += Width;
				_offset = _hash + _index;

				if constexpr (PowerOfTwo)
					_offset &= (_size - 1);
				else
					_offset %= _size;
			}

			void advance(int i)
			{
				_index += (Width * i);
				_offset = _hash + _index;

				if constexpr (PowerOfTwo)
					_offset &= (_size - 1);
				else
					_offset %= _size;
			}

			auto index() const { return _index; }
			auto hash() const { return _hash; }
			auto offset() const { return _offset; }
			auto offset(int i) const
			{
				if constexpr (PowerOfTwo)
					return (_hash + _index + i) & (_size - 1);
				else
					return (_hash + _index + i) % _size;
			}

		private:
			size_t _hash;
			size_t _size;
			size_t _offset;
			int _index;
		};

	}

	
	// use open + stable addressing
	// and linear probing
	template<
		typename T, 
		typename Hash = std::hash<T>, 
		typename Equal = std::equal_to<T>
	>
	class simple_set
	{
		struct set_node
		{
			using value_type = T;

			template<typename... TArgs>
			set_node(size_t hash, TArgs&&... args)
			{
				construct(hash, std::forward<TArgs>(args)...);
			}

			~set_node()
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
		using array_type = simple_buffer<set_node*>;
		using array_type_reference = array_type&;
		using const_array_type_reference = const array_type&;
		using key_type = T;
		using size_type = size_t;
		using growth_policy = detail::prime_growth;
		static constexpr bool using_power_of_two_growth = std::is_same_v<growth_policy, detail::power_of_two_growth>;
		using probe_type = detail::linear_probe_seq<1, using_power_of_two_growth>;

	private:
		template<typename... TArgs>
		static bool _emplace_into_slot(size_t hash, set_node*& slot, TArgs&&... args)
		{
			if (slot == nullptr)
			{
				slot = new set_node(hash, std::forward<TArgs>(args)...);
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
			probe_type seq(Hash{}(key), arr.capacity());
			while (arr[seq.offset()] != nullptr && arr[seq.offset()]->alive()) seq.next();

			return _emplace_into_slot(seq.hash(), arr[seq.offset()], key);
		}

		static bool _try_insert_into_empty(array_type_reference arr, set_node* node)
		{
			if (node == nullptr || !node->alive()) return false;

			// assume empty table has no deletions, so we dont check here
			probe_type seq(node->hash(), arr.capacity());
			while (arr[seq.offset()] != nullptr) seq.next();

			arr[seq.offset()] = node;

			return true;
		}

		static bool _try_erase_from(array_type_reference arr, const key_type& key)
		{
			probe_type seq(Hash{}(key), arr.capacity());
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
			probe_type seq(Hash{}(key), arr.capacity());
			while (arr[seq.offset()] != nullptr)
			{
				if (arr[seq.offset()]->alive() && Equal{}(arr[seq.offset()]->get(), key))
					return true;

				seq.next();
			}

			return false;
		}

		bool _should_grow()
		{
			return load_factor() > 0.75f;
		}

		void _grow_table()
		{
			auto new_size = growth_policy{}.next(capacity());
			array_type new_arr(new_size, nullptr);
			_load = 0;
			for (auto& ptr : _arr)
			{
				if (!ptr) continue;

				if (!ptr->alive())
					delete ptr;

				if(_try_insert_into_empty(new_arr, ptr))
					++_load;
			}
			_arr = std::move(new_arr);
		}

	public:
		simple_set() : _arr(8, nullptr) {}

		explicit simple_set(size_type size_hint)
			: _arr(growth_policy{}.start(size_hint), nullptr) {}
		
		template<typename It>
		simple_set(
			It first, It last, 
			size_type size_hint = 0
		) : _arr(growth_policy{}.start(size_hint), nullptr)
		{
			for (; first != last; ++first)
				insert(static_cast<key_type>(*first));
		}

		template <typename TArg>
		simple_set(
			std::initializer_list<TArg> il, 
			size_type size_hint = 0
		) : simple_set(il.begin(), il.end(), growth_policy{}.start(size_hint))	{}


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





}

#endif
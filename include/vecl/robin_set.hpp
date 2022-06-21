
#ifndef VECL_ROBIN_SET_H
#define VECL_ROBIN_SET_H

#include "config/config.h"

#include "simple_set.hpp"

namespace vecl
{
	namespace detail
	{
		template<typename T, typename Hash, typename Equal, bool StoreHash>
		class robin_set_impl
		{
		public:
			using key_type = T;
			using size_type = size_t;
			using distance_type = uint16_t;
			using growth_policy = power_of_two_growth;
			using probe_type = linear_probe_seq<1, true>;
			static constexpr bool is_inlined = false;

		private:
			template<bool>
			struct set_node;

			template<>
			struct set_node<true>
			{
				using value_type = T;

				template<typename... TArgs>
				set_node(size_t hash, distance_type probe_count, TArgs&&... args)
					: _hash(hash), _value(std::forward<TArgs>(args)...), probes(probe_count)
				{
				}

				VECL_NODISCARD auto hash() const { return _hash; }

				VECL_NODISCARD value_type& get() { return _value; }

				VECL_NODISCARD const value_type& get() const { return _value; }


			private:
				size_t _hash = 0;
				T _value;
			public:
				distance_type probes = 0;
			};

			template<>
			struct set_node<false>
			{
				using value_type = T;

				template<typename... TArgs>
				set_node(size_t, distance_type probe_count, TArgs&&... args)
					: _value(std::forward<TArgs>(args)...), probes(probe_count)
				{
				}

				VECL_NODISCARD auto hash() const { return 0; }

				VECL_NODISCARD value_type& get() { return _value; }

				VECL_NODISCARD const value_type& get() const { return _value; }


			private:
				T _value;
			public:
				distance_type probes = 0;
			};

		public:
			using node_type = set_node<StoreHash>;
			using array_type = simple_buffer<node_type*>;
			using array_type_reference = array_type&;
			using const_array_type_reference = const array_type&;
			


		private:
			/*template<typename... TArgs>
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

			}*/

			static bool _compare_hash(size_t hash, node_type* node)
			{
				if constexpr (StoreHash)
					return hash == node->hash();
				else
					return true;
			}

			static bool _insert_into(distance_type& max_probes, array_type_reference arr, const key_type& key)
			{
				probe_type seq(Hash{}(key), arr.capacity());
				distance_type probe_count = 0; // we do not count the first probe
				while (true)
				{
					// case 1: found empty slot -> just put
					// case 2: found a slot that my probe > their probe -> override the slot and reinsert their node
					if (arr[seq.offset()] == nullptr)
					{
						arr[seq.offset()] = new node_type(seq.hash(), probe_count, key);
						max_probes = std::max(probe_count, max_probes);
						return true;
					}
					else if (Equal{}(arr[seq.offset()]->get(), key))
					{
						// coliision but its just us
						return false;
					}
					else if (probe_count > arr[seq.offset()]->probes)
					{
						// steal the slot
						auto node = arr[seq.offset()];
						arr[seq.offset()] = new node_type(seq.hash(), probe_count, key);

						// call the recursive fn to reinsert for us
						_insert_into(max_probes, arr, node);
						max_probes = std::max(probe_count, max_probes);

						return true;
					}

					++probe_count;
					seq.next();
				}

				VECL_ASSERT(false, "Logic error occured!");

				return false;
			}

			static void _insert_into(distance_type& max_probes, array_type_reference arr, node_type* node)
			{
				size_t hash = 0;
				if constexpr (StoreHash)
					hash = node->hash();
				else
					hash = Hash{}(node->get());

				probe_type seq(hash, arr.capacity());
				seq.advance((int)node->probes);
				while (true)
				{
					if (arr[seq.offset()] == nullptr)
					{
						arr[seq.offset()] = node;
						max_probes = std::max(node->probes, max_probes);
						return;
					}
					//else if (Equal{}(arr[seq.offset()]->get(), key)) // there should never be this case
					//{
					//	// coliision but its just us
					//	return false;
					//}
					else if (node->probes > arr[seq.offset()]->probes)
					{
						// steal the slot
						auto temp = arr[seq.offset()];
						arr[seq.offset()] = node;

						// call the recursive fn to reinsert for us
						_insert_into(max_probes, arr, temp);
						max_probes = std::max(node->probes, max_probes);

						return;
					}

					++node->probes;
					seq.next();
				}

				VECL_ASSERT(false, "Logic error occured!");
			}

			static void _fill_slot(probe_type& seq, array_type_reference arr)
			{
				// assume slot is cleared

				auto next = seq.offset(1);
				while (arr[next] != nullptr && arr[next]->probes != 0)
				{
					--arr[next]->probes;
					arr[seq.offset()] = arr[next];
					seq.next();
					next = seq.offset(1);
				}

				arr[seq.offset()] = nullptr;
			}

			static bool _try_erase_from(distance_type max_probes, array_type_reference arr, const key_type& key)
			{
				probe_type seq(Hash{}(key), arr.capacity());
				auto t = max_probes + 1;
				while (t--)
				{
					if (arr[seq.offset()] == nullptr)
					{
						return false;
					}
					else if (Equal{}(arr[seq.offset()]->get(), key)) // there should never be this case
					{
						// clear this guy, then keep pulling the next guy to fill in the empty slot
						delete arr[seq.offset()];
						_fill_slot(seq, arr);
						return true;
					}

					seq.next();
				}
				return false;
			}

			static bool _contained_in(distance_type max_probes, const_array_type_reference arr, const key_type& key)
			{
				probe_type seq(Hash{}(key), arr.capacity());
				auto t = max_probes + 1;
				
				while (t--)
				{
					auto node = arr[seq.offset()];
					if (node == nullptr)
					{
						return false;
					}
					else if (
						_compare_hash(seq.hash(), node) &&
						Equal{}(node->get(), key)
						)
					{
						// coliision but its just us
						return true;
					}

					seq.next();
				}

				return false;
			}

			void _insert_into(array_type_reference arr, node_type* node)
			{
				probe_type seq(node->hash(), arr.capacity());
				seq.advance((int)node->probes);
				while (true)
				{
					if (arr[seq.offset()] == nullptr)
					{
						arr[seq.offset()] = node;
						_max_probes = std::max(node->probes, _max_probes);
						return;
					}
					//else if (Equal{}(arr[seq.offset()]->get(), key)) // there should never be this case
					//{
					//	// coliision but its just us
					//	return false;
					//}
					else if (node->probes > arr[seq.offset()]->probes)
					{
						// steal the slot
						auto temp = arr[seq.offset()];
						arr[seq.offset()] = node;

						// call the recursive fn to reinsert for us
						_insert_into(arr, temp);
						_max_probes = std::max(node->probes, _max_probes);

						return;
					}

					++node->probes;
					seq.next();
				}

				VECL_ASSERT(false, "Logic error occured!");
			}

			bool _insert(const key_type& key)
			{
				probe_type seq(Hash{}(key), capacity());
				size_t probe_count = 0; // we do not count the first probe
				while (true)
				{
					// case 1: found empty slot -> just put
					// case 2: found a slot that my probe > their probe -> override the slot and reinsert their node
					if (_arr[seq.offset()] == nullptr)
					{
						_arr[seq.offset()] = new node_type(seq.hash(), probe_count, key);
						_max_probes = std::max(probe_count, _max_probes);
						return true;
					}
					else if (Equal{}(_arr[seq.offset()]->get(), key))
					{
						// coliision but its just us
						return false;
					}
					else if (probe_count > _arr[seq.offset()]->probes)
					{
						// steal the slot
						auto node = _arr[seq.offset()];
						_arr[seq.offset()] = new set_node(seq.hash(), probe_count, key);

						// call the recursive fn to reinsert for us
						_insert(node);
						_max_probes = std::max(probe_count, _max_probes);

						return true;
					}

					++probe_count;
					seq.next();
				}

				VECL_ASSERT(false, "Logic error occured!");

				return false;
			}

			void _insert(node_type* node)
			{
				size_t hash = 0;
				if constexpr (StoreHash)
					hash = node->hash();
				else
					hash = Hash{}(node->get());

				probe_type seq(hash, capacity());
				seq.advance((int)node->probes);
				while (true)
				{
					if (_arr[seq.offset()] == nullptr)
					{
						_arr[seq.offset()] = node;
						_max_probes = std::max(node->probes, _max_probes);
						return;
					}
					//else if (Equal{}(arr[seq.offset()]->get(), key)) // there should never be this case
					//{
					//	// coliision but its just us
					//	return false;
					//}
					else if (node->probes > _arr[seq.offset()]->probes)
					{
						// steal the slot
						auto temp = _arr[seq.offset()];
						_arr[seq.offset()] = node;

						// call the recursive fn to reinsert for us
						_insert(temp);
						_max_probes = std::max(node->probes, _max_probes);

						return;
					}

					++node->probes;
					seq.next();
				}

				VECL_ASSERT(false, "Logic error occured!");
			}

			bool _try_erase(const key_type& key)
			{
				probe_type seq(Hash{}(key), capacity());
				auto t = _max_probes + 1;
				while (t--)
				{
					if (_arr[seq.offset()] == nullptr)
					{
						return false;
					}
					else if (Equal{}(_arr[seq.offset()]->get(), key)) // there should never be this case
					{
						// clear this guy, then keep pulling the next guy to fill in the empty slot
						delete _arr[seq.offset()];

						auto next = seq.offset(1);
						while (_arr[next] != nullptr && _arr[next]->probes != 0)
						{
							--_arr[next]->probes;
							_arr[seq.offset()] = _arr[next];
							seq.next();
							next = seq.offset(1);
						}

						_arr[seq.offset()] = nullptr;
						return true;
					}

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
				distance_type new_max_probes = 0;
				auto new_size = growth_policy{}.next(capacity());
				array_type new_arr(new_size, nullptr);
				for (auto& ptr : _arr)
				{
					if (ptr)
					{
						ptr->probes = 0; // reset probe_count
						_insert_into(new_max_probes, new_arr, ptr);
					}
				}
				_arr = std::move(new_arr);
				_max_probes = new_max_probes;
			}

		public:
			robin_set_impl() : _arr(8, nullptr) {}

			explicit robin_set_impl(size_type size_hint)
				: _arr(growth_policy{}.start(size_hint), nullptr) {}

			template<typename It>
			robin_set_impl(
				It first, It last,
				size_type size_hint = 0
			) : _arr(growth_policy{}.start(size_hint), nullptr)
			{
				for (; first != last; ++first)
					insert(static_cast<key_type>(*first));
			}

			template <typename TArg>
			robin_set_impl(
				std::initializer_list<TArg> il,
				size_type size_hint = 0
			) : robin_set_impl(il.begin(), il.end(), growth_policy{}.start(size_hint)) {}


			~robin_set_impl()
			{
				clear();
			}


			bool insert(const key_type& key)
			{
				if (_should_grow())
					_grow_table();

				if (_insert_into(_max_probes, _arr, key))
				{
					++_size;
					return true;
				}

				return false;
			}

			bool erase(const key_type& key)
			{
				if (_try_erase_from(_max_probes, _arr, key))
					return --_size, true;
				return false;
			}

			auto contains(const key_type& key) const
			{
				return _contained_in(_max_probes, _arr, key);
			}

			size_t count(const key_type& key) const
			{
				return _contained_in(_max_probes, _arr, key) ? 1 : 0;
			}

			auto size() const { return _size; }

			auto capacity() const { return _arr.capacity(); }

			auto load_factor() const { return 1.f * size() / capacity(); }

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
				_max_probes = 0;
			}

			friend std::ostream& operator<<(std::ostream& os, const robin_set_impl& set)
			{
				for (auto i = 0; i < (int)set._arr.capacity(); ++i)
				{
					if (i && i % 20 == 0)
						os << "\n";

					os << "[";

					if (set._arr[i] == nullptr)
						os << std::setw(2) << "  ";
					else
						os << std::setw(2) << set._arr[i]->get();

					os << "]";
				}
				return os;
			}

		private:
			array_type _arr;
			size_type _size = 0;
			distance_type _max_probes = 0;
		};

		template<typename T, typename Hash, typename Equal, bool StoreHash>
		class inline_robin_set_impl
		{
		public:
			using key_type = T;
			using size_type = size_t;
			using distance_type = uint16_t;
			using growth_policy = power_of_two_growth;
			using probe_type = linear_probe_seq<1, true>;
			static constexpr bool is_inlined = true;

		private:
			template<bool>
			struct inline_set_node;

			template<>
			struct inline_set_node<true>
			{
				using value_type = T;

				inline_set_node() = default;

				template<typename... TArgs>
				inline_set_node(size_t hash, distance_type probe_count, TArgs&&... args)
				{
					construct(hash, probe_count, std::forward<TArgs>(args)...);
				}

				inline_set_node(const inline_set_node& rhs) = delete;
				inline_set_node& operator=(const inline_set_node& rhs) = delete;

				inline_set_node(inline_set_node&& rhs) noexcept(false)
				{
					operator=(std::move(rhs));
				}

				inline_set_node& operator=(inline_set_node&& rhs) noexcept(false)
				{
					if (_alive && rhs._alive)
					{
						std::swap(get(), rhs.get());
					}
					else if (_alive)
					{
						rhs.construct(_hash, probes, std::move(get()));
						destroy();
					}
					else if (rhs._alive) VECL_LIKELY
					{
						construct(rhs._hash, rhs.probes, std::move(rhs.get()));
						rhs.destroy();
					}

					return *this;
				}

				~inline_set_node()
				{
					destroy();
				}

				template<typename... TArgs>
				void construct(size_t hash, distance_type probe_count, TArgs&&... args)
				{
					if (!_alive)
					{
						_hash = hash;
						probes = probe_count;
						std::construct_at(data(), std::forward<TArgs>(args)...);
					}

					_alive = true;
				}

				void destroy()
				{
					if constexpr (!std::is_trivially_destructible_v<T>)
						if (_alive)
							std::destroy_at(data());

					probes = 0;
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
			public:
				distance_type probes = 0;
			private:
				bool _alive = false;
			};

			template<>
			struct inline_set_node<false>
			{
				using value_type = T;

				inline_set_node() = default;

				template<typename... TArgs>
				inline_set_node(size_t, distance_type probe_count, TArgs&&... args)
				{
					construct(probe_count, std::forward<TArgs>(args)...);
				}

				inline_set_node(const inline_set_node& rhs) = delete;
				inline_set_node& operator=(const inline_set_node& rhs) = delete;

				inline_set_node(inline_set_node&& rhs) noexcept(false)
				{
					operator=(std::move(rhs));
				}

				inline_set_node& operator=(inline_set_node&& rhs) noexcept(false)
				{
					if (_alive && rhs._alive)
					{
						std::swap(get(), rhs.get());
					}
					else if (_alive)
					{
						rhs.construct(probes, std::move(get()));
						destroy();
					}
					else if (rhs._alive) VECL_LIKELY
					{
						construct(rhs.probes, std::move(rhs.get()));
						rhs.destroy();
					}

					return *this;
				}

				~inline_set_node()
				{
					destroy();
				}

				template<typename... TArgs>
				void construct(distance_type probe_count, TArgs&&... args)
				{
					if (!_alive)
					{
						probes = probe_count;
						std::construct_at(data(), std::forward<TArgs>(args)...);
					}

					_alive = true;
				}

				void destroy()
				{
					if constexpr (!std::is_trivially_destructible_v<T>)
						if (_alive)
							std::destroy_at(data());

					probes = 0;
					_alive = false;
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


				alignas(T) storage_type _value;
			public:
				distance_type probes = 0;
			private:
				bool _alive = false;
			};

		public:
			using node_type = inline_set_node<StoreHash>;
			using array_type = simple_buffer<node_type>;
			using array_type_reference = array_type&;
			using const_array_type_reference = const array_type&;

		private:

			static bool _compare_hash(size_t hash, const node_type& node)
			{
				if constexpr (StoreHash)
					return hash == node.hash();
				else
					return true;
			}

			static void _insert_into(distance_type& max_probes, array_type_reference arr, node_type&& node)
			{
				size_t hash = 0;
				if constexpr (StoreHash)
					hash = node.hash();
				else
					hash = Hash{}(node.get());

				probe_type seq(hash, arr.capacity());
				seq.advance((int)node.probes);
				while (true)
				{
					if (!arr[seq.offset()].alive())
					{
						max_probes = std::max(node.probes, max_probes);
						arr[seq.offset()] = std::move(node);
						return;
					}
					else if (node.probes > arr[seq.offset()].probes)
					{
						// steal the slot
						auto temp = std::move(arr[seq.offset()]);
						max_probes = std::max(node.probes, max_probes);
						arr[seq.offset()] = std::move(node);

						// call the recursive fn to reinsert for us
						_insert_into(max_probes, arr, std::move(temp));

						return;
					}

					++node.probes;
					seq.next();
				}

				VECL_ASSERT(false, "Logic error occured!");
			}

			static bool _insert_into(distance_type& max_probes, array_type_reference arr, const key_type& key)
			{
				probe_type seq(Hash{}(key), arr.capacity());
				distance_type probe_count = 0; // we do not count the first probe
				while (true)
				{
					// case 1: found empty slot -> just put
					// case 2: found a slot that my probe > their probe -> override the slot and reinsert their node
					if (!arr[seq.offset()].alive())
					{
						if constexpr (StoreHash)
							arr[seq.offset()].construct(seq.hash(), probe_count, key);
						else
							arr[seq.offset()].construct(probe_count, key);

						max_probes = std::max(probe_count, max_probes);
						return true;
					}
					else if (Equal{}(arr[seq.offset()].get(), key))
					{
						// coliision but its just us
						return false;
					}
					else if (probe_count > arr[seq.offset()].probes)
					{
						// steal the slot
						auto node = std::move(arr[seq.offset()]);
						if constexpr (StoreHash)
							arr[seq.offset()].construct(seq.hash(), probe_count, key);
						else
							arr[seq.offset()].construct(probe_count, key);

						// call the recursive fn to reinsert for us
						_insert_into(max_probes, arr, std::move(node));
						max_probes = std::max(probe_count, max_probes);

						return true;
					}

					++probe_count;
					seq.next();
				}

				VECL_ASSERT(false, "Logic error occured!");

				return false;
			}

			static bool _try_erase(distance_type max_probes, array_type_reference arr, const key_type& key)
			{
				probe_type seq(Hash{}(key), arr.capacity());
				auto t = max_probes + 1;
				while (t--)
				{
					if (!arr[seq.offset()].alive())
					{
						return false;
					}
					else if (Equal{}(arr[seq.offset()].get(), key)) // there should never be this case
					{
						// clear this guy, then keep pulling the next guy to fill in the empty slot
						arr[seq.offset()].destroy();

						auto next = seq.offset(1);
						while (arr[next].alive() && arr[next].probes != 0)
						{
							--arr[next].probes;
							arr[seq.offset()] = std::move(arr[next]);
							seq.next();
							next = seq.offset(1);
						}
						return true;
					}

					seq.next();
				}
				return false;
			}

			static bool _contained_in(distance_type max_probes, const_array_type_reference arr, const key_type& key)
			{
				probe_type seq(Hash{}(key), arr.capacity());
				auto t = max_probes + 1;
				while (t--)
				{
					auto& node = arr[seq.offset()];
					if (!node.alive())
					{
						return false;
					}
					else if (
						//_compare_hash(seq.hash(), node) &&
						Equal{}(node.get(), key))
					{
						// coliision but its just us
						return true;
					}

					seq.next();
				}

				return false;
			}

			void _insert_into(array_type_reference arr, node_type&& node)
			{
				size_t hash = 0;
				if constexpr (StoreHash)
					hash = node.hash();
				else
					hash = Hash{}(node.get());

				probe_type seq(hash, arr.capacity());
				seq.advance((int)node.probes);
				while (true)
				{
					if (!arr[seq.offset()].alive())
					{
						arr[seq.offset()] = std::move(node);
						_max_probes = std::max(node.probes, _max_probes);
						return;
					}
					else if (node.probes > arr[seq.offset()].probes)
					{
						// steal the slot
						auto temp = std::move(arr[seq.offset()]);
						arr[seq.offset()] = std::move(node);

						// call the recursive fn to reinsert for us
						_insert_into(arr, std::move(temp));
						_max_probes = std::max(node.probes, _max_probes);

						return;
					}

					++node.probes;
					seq.next();
				}

				VECL_ASSERT(false, "Logic error occured!");
			}

			bool _insert(const key_type& key)
			{
				probe_type seq(Hash{}(key), capacity());
				size_t probe_count = 0; // we do not count the first probe
				while (true)
				{
					// case 1: found empty slot -> just put
					// case 2: found a slot that my probe > their probe -> override the slot and reinsert their node
					if (!_arr[seq.offset()].alive())
					{

						if constexpr (StoreHash)
							_arr[seq.offset()].construct(seq.hash(), probe_count, key);
						else
							_arr[seq.offset()].construct(probe_count, key);

						_max_probes = std::max(probe_count, _max_probes);
						return true;
					}
					else if (Equal{}(_arr[seq.offset()].get(), key))
					{
						// coliision but its just us
						return false;
					}
					else if (probe_count > _arr[seq.offset()].probes)
					{
						// steal the slot
						auto node = std::move(_arr[seq.offset()]);

						if constexpr (StoreHash)
							_arr[seq.offset()].construct(seq.hash(), probe_count, key);
						else
							_arr[seq.offset()].construct(probe_count, key);

						// call the recursive fn to reinsert for us
						_insert(std::move(node));
						_max_probes = std::max(probe_count, _max_probes);

						return true;
					}

					++probe_count;
					seq.next();
				}

				VECL_ASSERT(false, "Logic error occured!");

				return false;
			}

			void _insert(node_type&& node)
			{
				size_t hash = 0;
				if constexpr (StoreHash)
					hash = node.hash();
				else
					hash = Hash{}(node.get());

				probe_type seq(hash, capacity());
				seq.advance((int)node.probes);
				while (true)
				{
					if (!_arr[seq.offset()].alive())
					{
						_max_probes = std::max(node.probes, _max_probes);
						_arr[seq.offset()] = std::move(node);
						return;
					}
					//else if (Equal{}(arr[seq.offset()]->get(), key)) // there should never be this case
					//{
					//	// coliision but its just us
					//	return false;
					//}
					else if (node.probes > _arr[seq.offset()].probes)
					{
						_max_probes = std::max(node.probes, _max_probes);
						// steal the slot
						auto temp = std::move(_arr[seq.offset()]);
						_arr[seq.offset()] = std::move(node);

						// call the recursive fn to reinsert for us
						_insert(std::move(temp));

						return;
					}

					++node.probes;
					seq.next();
				}

				VECL_ASSERT(false, "Logic error occured!");
			}

			bool _try_erase(const key_type& key)
			{
				probe_type seq(Hash{}(key), capacity());
				auto t = _max_probes + 1;
				while (t--)
				{
					if (!_arr[seq.offset()].alive())
					{
						return false;
					}
					else if (Equal{}(_arr[seq.offset()].get(), key)) // there should never be this case
					{
						// clear this guy, then keep pulling the next guy to fill in the empty slot
						_arr[seq.offset()].destroy();

						auto next = seq.offset(1);
						while (_arr[next].alive() && _arr[next].probes != 0)
						{
							--_arr[next].probes;
							_arr[seq.offset()] = std::move(_arr[next]);
							seq.next();
							next = seq.offset(1);
						}

						//_arr[seq.offset()] = nullptr;
						return true;
					}

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
				distance_type new_max_probes = 0;
				auto new_size = growth_policy{}.next(capacity());
				array_type new_arr(new_size);
				for (auto& ptr : _arr)
				{
					if (ptr.alive())
					{
						ptr.probes = 0; // reset probe_count
						_insert_into(new_max_probes, new_arr, std::move(ptr));
					}
				}
				_arr = std::move(new_arr);
				_max_probes = new_max_probes;
			}

		public:
			inline_robin_set_impl() : _arr(8) {}

			explicit inline_robin_set_impl(size_type size_hint)
				: _arr(growth_policy{}.start(size_hint)) {}

			template<typename It>
			inline_robin_set_impl(
				It first, It last,
				size_type size_hint = 0
			) : _arr(growth_policy{}.start(size_hint))
			{
				for (; first != last; ++first)
					insert(static_cast<key_type>(*first));
			}

			template <typename TArg>
			inline_robin_set_impl(
				std::initializer_list<TArg> il,
				size_type size_hint = 0
			) : robin_set_impl(il.begin(), il.end(), growth_policy{}.start(size_hint)) {}


			~inline_robin_set_impl()
			{
				clear();
			}


			bool insert(const key_type& key)
			{
				if (_should_grow())
					_grow_table();

				if (_insert_into(_max_probes, _arr, key))
				{
					++_size;
					return true;
				}

				return false;
			}

			bool erase(const key_type& key)
			{
				if (_try_erase(_max_probes, _arr, key))
					return --_size, true;
				return false;
			}

			auto contains(const key_type& key) const
			{
				return _contained_in(_max_probes, _arr, key);
			}

			size_t count(const key_type& key) const
			{
				return _contained_in(_max_probes, _arr, key) ? 1 : 0;
			}

			auto size() const { return _size; }

			auto capacity() const { return _arr.capacity(); }

			auto load_factor() const { return 1.f * size() / capacity(); }

			void clear()
			{
				for (auto& ptr : _arr)
				{
					ptr.destroy();
				}

				_size = 0;
				_max_probes = 0;
			}

			friend std::ostream& operator<<(std::ostream& os, const inline_robin_set_impl& set)
			{
				for (auto i = 0; i < (int)set._arr.capacity(); ++i)
				{
					if (i && i % 20 == 0)
						os << "\n";

					os << "[";

					if (!set._arr[i].alive())
						os << std::setw(2) << "  ";
					else
						os << std::setw(2) << set._arr[i].get();

					os << "]";
				}
				return os;
			}

		private:
			array_type _arr;
			size_type _size = 0;
			distance_type _max_probes = 0;
		};

		template<typename T, typename Hash, typename Equal, bool StoreHash, bool Inlined>
		struct robin_selector
		{
			static constexpr bool store_hash = StoreHash || !std::is_arithmetic_v<T>;
			static constexpr bool is_inlined = Inlined; // (std::is_arithmetic_v<T>&& std::is_same_v<Hash, std::hash<T>>);

			using type = std::conditional_t<is_inlined,
				inline_robin_set_impl<T, Hash, Equal, store_hash>,
				robin_set_impl<T, Hash, Equal, store_hash>>;
		};

		template<typename T, typename Hash, typename Equal, bool StoreHash, bool Inlined>
		using robin_selector_t = typename robin_selector<T, Hash, Equal, StoreHash, Inlined>::type;
	}

	template<
		typename T,
		typename Hash = std::hash<T>,
		typename Equal = std::equal_to<T>,
		bool StoreHash = false,
		bool Inlined = true 
	>
	class robin_set : public detail::robin_selector_t<T, Hash, Equal, StoreHash, Inlined> {};




} // namespace vecl

#endif
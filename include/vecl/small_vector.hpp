#ifndef VECL_SMALL_VECTOR_H
#define VECL_SMALL_VECTOR_H

#include "config/config.h"

#include <iterator> // advance, distance
#include <stdexcept> // length_error
#include <memory> // uninitialized_move, uninitialized_copy 

namespace vecl
{

	/**
	 * @brief Buffer object. We make sure the buffer can store at least
	 * a size_t so that we can scribble some data on it later on.
	 */
	template<typename T, size_t N>
	struct small_vector_buffer
	{
		alignas(T) char _buffer[std::max(sizeof(size_t), N * sizeof(T))];
	};

	/**
	 * @brief A type-erased object for small_vectors. We erase the small
	 * buffer size from the original small_vector to reduce code duplication
	 * and allow nice implicit casting.
	 *
	 * @tparam T Type of element.
	 */
	template <typename T>
	class small_vector_ref
	{
	public:
		/**
		 * @note TYPE TRAITS
		 */
		using value_type = T;
		using size_type = size_t;
		using difference_type = ptrdiff_t;
		using reference = value_type&;
		using const_reference = const value_type&;
		using pointer = value_type*;
		using const_pointer = const value_type*;

		using iterator = pointer;
		using const_iterator = const iterator;
		using reverse_iterator = typename std::reverse_iterator<iterator>;
		using const_reverse_iterator = const reverse_iterator;


	private:
		/**
		 * @brief A helper struct that models the size and alignment of the
		 * small_vector_ref object.
		 */
		struct _small_vec_ref_align_and_size 
		{
			alignas(small_vector_ref<T>) 
			char _ref[sizeof(small_vector_ref<T>)];
			alignas(T) char _first[sizeof(T)];
		};

		/**
		 * @brief Gets the first element in the stack buffer by finding
		 * the offset of the _first member in _small_vec_ref_align_and_size
		 * and offseting it from this pointer.
		 */
		constexpr void* _get_first() const {
			return const_cast<void*>(reinterpret_cast<const void*>(
				reinterpret_cast<const char*>(this) +
				offsetof(_small_vec_ref_align_and_size, _first)));
		}

		/**
		 * @brief Destroys the elements in a given range.
		 */
		constexpr void _destroy_range(T* s, T* e) VECL_NOEXCEPT
		{
			if constexpr (!std::is_trivially_destructible<T>::value)
				while (s != e) (--e)->~value_type();
		}

		/**
		 * @brief Gets the new capacity of the vector after it grows.
		 * 
		 * @throw std::length_error if either our min_size is too much
		 * or if our capacity is already at max.
		 */
		constexpr size_type _get_new_capacity_after_grow(size_type min_size)
		{
			// if requested min amount of ele exceeds max
			if (min_size > max_size())
				throw std::length_error(
					"min_size requested exceeds small_vector's max_size"
				);

			// if capacity already at max
			if(capacity() == max_size())
				throw std::length_error(
					"small_vector's capacity already at max_size"
				);

			// clamp between min and max so we have 
			// sufficient for min yet do not exceed max
			return std::min(std::max(min_size, 2 * capacity() + 1), max_size());
		}

		/**
		 * @brief Allocation function.
		 * 
		 * @todo Modify for allocator awareness
		 */
		constexpr void* _allocate(size_t size) const
		{
			return malloc(size);
		}

		/**
		 * @brief Dellocation function.
		 * 
		 * @todo Modify for allocator awareness
		 */
		constexpr void _deallocate(void* ptr, [[maybe_unused]] size_t size) const
		{
			free(ptr);
		}

		/**
		 * @brief Set new capacity for the grow, then allocate the new buffer
		 * for the grow, then return the buffer.
		 */
		constexpr T* _allocate_for_grow(size_t min_size, size_t& new_capacity)
		{
			new_capacity = _get_new_capacity_after_grow(min_size);
			return static_cast<T*>(_allocate(new_capacity * sizeof(T)));
		}


		/**
		 * @brief If we are not small, deallocate the current buffer.
		 * If we are small, then we scribble the original capacity on the
		 * buffer, so we can retrieve it later if we reset.
		 */
		constexpr void _deallocate_current_buffer()
		{
			if (!is_small())
				_deallocate(_buffer, _capacity);
			else // we scribble our og capacity on the storage buffer
				*reinterpret_cast<size_type*>(_buffer) = _capacity;
		}

		/**
		 * @brief Deallocate the current buffer then set it to the new buffer.
		 * Update the capacity.
		 */
		constexpr void _take_allocation_for_grow(
			T* new_buffer, 
			size_type& new_capacity
		) 
		{
			_deallocate_current_buffer();

			_buffer = new_buffer;
			_capacity = new_capacity;
		}

		/**
		 * @brief Move our old stuff to the new buffer, then we destroy the old
		 * stuff.
		 */
		constexpr void _move_for_grow(T* new_buffer)
		{
			std::uninitialized_move(begin(), end(), new_buffer);
			_destroy_range(begin(), end());
		}

		/**
		 * @brief Grow our vector then assign ele_n number of ele
		 */
		constexpr void _grow_and_assign(size_t ele_n, const T& ele) 
		{
			// allocate and fill our new buffer
			size_t new_capacity;
			T* new_buffer = _allocate_for_grow(ele_n, new_capacity);
			std::uninitialized_fill_n(new_buffer, ele_n, ele);
			
			// destroy all of the current objects
			_destroy_range(begin(), end());
			// then take the new buffer and update our size
			_take_allocation_for_grow(new_buffer, new_capacity);
			_size = ele_n;
		}

		/**
		 * @brief Grow our vector then assign ele_n number of default T
		 */
		constexpr void _grow_and_assign(size_t ele_n) 
		{
			// allocate and fill our new buffer
			size_t new_capacity;
			T* new_buffer = _allocate_for_grow(ele_n, new_capacity);
			std::uninitialized_default_construct_n(new_buffer, ele_n);

			// destroy all of the current objects
			_destroy_range(begin(), end());
			// then take the new buffer and update our size
			_take_allocation_for_grow(new_buffer, new_capacity);
			_size = ele_n;
		}

		/**
		 * @brief Grow our vector to at least min_size elements
		 */
		constexpr void _grow(size_t min_size = 0) 
		{
			// allocate and move to our new buffer
			size_t new_capacity;
			T* new_buffer = _allocate_for_grow(min_size, new_capacity);
			_move_for_grow(new_buffer);
			// then take the new buffer
			_take_allocation_for_grow(new_buffer, new_capacity);
		}

		/** 
		 * @brief Grow our vector then emplace our arguments
		 */
		template <typename... Args> 
		constexpr T& _grow_and_emplace_back(Args&&... args)
		{
			// allocate
			size_t new_capacity;
			T* new_buffer = _allocate_for_grow(1, new_capacity);
			// we emplace first because we cannot check if args contain
			// a reference in buffer
			::new ((void*)(new_buffer + size())) T(std::forward<Args>(args)...);
			// we move after since we have emplaced the object already
			_move_for_grow(new_buffer);
			// take the new buffer and update the size
			_take_allocation_for_grow(new_buffer, new_capacity);
			++_size;
			return back();
		}

		
		/**
		 * @brief Reserve for at least ele_n extra elements, and make sure
		 * our ele is not invalidated.
		 */
		constexpr const T* _reserve_for_param(size_t ele_n, const T& ele)
		{
			// if no need to grow we can just return ele
			// since it will never be invalidated
			size_t new_size = size() + ele_n;
			if (new_size <= capacity())
				return &ele;

			// if ele is a reference in the buffer
			if (_is_reference_in_buffer(&ele))
			{
				// change ele to an index before growing
				size_t ele_i = &ele - begin();
				_grow(new_size);
				// then return an invalidated version of ele
				return begin() + ele_i;
			}

			// else we can just grow and return ele
			_grow(new_size);
			return &ele;
		}

		/**
		 * @brief Reserve for at least ele_n extra elements, and make sure
		 * our ele is not invalidated. Overloaded for non-const references
		 * @todo Refactor for less code duplication
		 */
		constexpr T* _reserve_for_param(size_t ele_n, T& ele)
		{
			// if no need to grow we can just return ele
			// since it will never be invalidated
			size_t new_size = size() + ele_n;
			if (new_size <= capacity())
				return &ele;

			// if ele is a reference in the buffer
			if (_is_reference_in_buffer(&ele))
			{
				// change ele to an index before growing
				size_t ele_i = &ele - begin();
				_grow(new_size);
				// then return an invalidated version of ele
				return begin() + ele_i;
			}

			// else we can just grow and return ele
			_grow(new_size);
			return &ele;
		}


		/**
		 * @return true if ref is in the range.
		 */
		constexpr bool _is_reference_in_range(
			const T* ref, 
			const T* from, 
			const T* to
		) const
		{
			std::less<> less;
			return !less(ref, from) && less(ref, to);
		}

		/**
		 * @return true if ref is in the buffer.
		 */
		constexpr bool _is_reference_in_buffer(const T* ref) const
		{
			return _is_reference_in_range(ref, begin(), end());
		}

		/**
		 * @return true if range is in the buffer.
		 */
		constexpr bool _is_range_in_buffer(const T* from, const T* to) const
		{
			std::less<> less;
			return !less(from, begin()) && !less(to, from) &&
				!less(end(), to);
		}

		/**
		 * @return true if ref is not invalidated after a clear.
		 */
		constexpr bool _is_valid_after_clear(const T* ref) const
		{
			return !_is_reference_in_buffer(ref);
		}

		/**
		 * @return true if will grow after adding ele_n elements
		 */
		constexpr bool _will_grow(size_t ele_n = 1) const
		{
			return (ele_n + _size) > capacity();
		}

		/**
		 * @brief Asserts that a range is still valid after a clear.
		 */
		constexpr void _assert_valid_after_clear(
			const T* from, 
			const T* to
		) const
		{
			if (from == to)	return;
			VECL_ASSERT(_is_valid_after_clear(from));
			VECL_ASSERT(_is_valid_after_clear(to));
		}

		/**
		 * @brief Overload to ignore assertions when It is not an iterator
		 * of the container.
		 */
		template <typename It>
		requires !std::is_same<std::remove_const_t<It>, T*>::value
			constexpr void _assert_valid_after_clear(It, It) const {}

			
		/**
		 * @note MEMBER FUNCTIONS
		 */
	protected:
		explicit small_vector_ref(size_t N)
			: _buffer(_get_first()), _size(0), _capacity(N)
		{
		}

	public:
		 /**
		  * @brief Deleted Constructors.
		  */
		constexpr small_vector_ref() = delete;
		constexpr small_vector_ref(const small_vector_ref&) = delete;
		constexpr small_vector_ref(small_vector_ref&&) = delete;

		/**
		* @brief Destructor.
		*/
		constexpr ~small_vector_ref()
		{
			if (!is_small())
				_deallocate(_buffer, _capacity);
		}

		/**
		 * @brief Copy-Assignment Operator.
		 */
		constexpr small_vector_ref& operator=(const small_vector_ref& other)
		{
			// if this is other we do nothing
			if (&other == this) return *this;

			// if our size is greater than other's
			if (size() >= other.size())
			{
				// no allocation needed
				// just overwrite the elements in other over our range
				auto new_end = std::copy(other.begin(), other.end(), begin());
				// then we destroy the extra that wasn't overwritten
				_destroy_range(new_end, end());
				// and update our size
				_size = other.size();
				return *this;
			}

			// if our capacity isn't enough
			if (capacity() < other.size())
			{
				// clear the current so we don't move elements that we are
				// going to destroy during our grow
				clear();
				// grow our vector to fit at least other.size() number of
				// elements
				_grow(other.size());
				// copy over to our newly allocated buffer
				std::uninitialized_copy(other.begin(), other.end(), begin());
				// and update our size
				_size = other.size();
				return *this;

			}
			
			// else it means we have enough memory but not enough intialized
			// elements
			// overwrite all existing elements
			std::copy(other.begin(), other.begin() + size(), begin());
			// then uninitialize copy the rest over
			std::uninitialized_copy(
				other.begin() + size(), 
				other.end(), 
				begin() + size());
			// and update our size
			_size = other.size();
			return *this;
		}

		/**
		 * @brief Move-Assignment Operator.
		 */
		constexpr small_vector_ref& operator=(
			small_vector_ref&& other
		) VECL_NOEXCEPT_IF(false)
		{
			// if this is other we do nothing
			if (&other == this) return *this;

			// if other is not small
			if (!other.is_small())
			{
				// then we can just steal their buffer
				// we destroy our elements and deallocate our buffer
				_destroy_range(begin(), end());
				_deallocate_current_buffer();
				// take other's members
				_buffer = other._buffer;
				_size = other.size();
				_capacity = other.capacity();
				
				// since other has no more buffer, we can reset it back to
				// small
				other._buffer = other._get_first();
				other._size = 0;
				// get back the og capacity from the stack storage
				other._capacity = *reinterpret_cast<size_t*>(other._buffer);

				return *this;
			}

			// if our size is greater than other's
			if (size() >= other.size())
			{
				// no allocation needed
				// just overwrite the elements in other over our range
				auto new_end = std::move(other.begin(), other.end(), begin());
				// then we destroy the extra that wasn't overwritten
				_destroy_range(new_end, end());
				// and update our size
				_size = other.size();
				// and clear other
				other.clear();
				return *this;
			}

			// if our capacity isn't enough
			if (capacity() < other.size())
			{
				// clear the current so we don't move elements that we are
				// going to destroy during our grow
				clear();
				// grow our vector to fit at least other.size() number of
				// elements
				_grow(other.size());
				// move over to our newly allocated buffer
				std::uninitialized_move(other.begin(), other.end(), begin());
				// and update our size
				_size = other.size();
				// and clear other
				other.clear();
				return *this;
			}

			// else it means we have enough memory but not enough intialized
			// elements
			// overwrite all existing elements
			std::move(other.begin(), other.begin() + size(), begin());
			// then uninitialize move the rest over
			std::uninitialized_move(
				other.begin() + size(),
				other.end(),
				begin() + size());

			// and update our size
			_size = other.size();
			// and clear other
			other.clear();
			return *this;
		}

		/**
		 * @brief Fill assign.
		 *
		 * @param ele_n Number of elements
		 * @param value Element to fill by const-reference
		 */
		constexpr void assign(
			size_type ele_n,
			const value_type& value
		)
		{
			// if our capacity isn't enough
			if (ele_n > capacity()) 
			{
				// grow and assign
				_grow_and_assign(ele_n, value);
				return;
			}

			// else just overwrite over the current elements
			std::fill_n(begin(), std::min(ele_n, size()), value);
			// and either construct more elements at the end
			if (ele_n > size())
				std::uninitialized_fill_n(end(), ele_n - size(), value);
			// or destroy elements at the end to have size() == ele_n
			else if (ele_n < size())
				_destroy_range(begin() + ele_n, end());

			// and update our size
			_size = ele_n;
		}

		/**
		 * @brief Default Fill assign.
		 *
		 * @param ele_n Number of elements
		 */
		constexpr void assign(
			size_type ele_n
		)
		{
			// if our capacity isn't enough
			if (ele_n > capacity()) 
			{
				// grow and assign
				_grow_and_assign(ele_n);
				return;
			}

			// else just overwrite over the current elements
			std::fill_n(begin(), std::min(ele_n, size()), value_type());
			// and either construct more elements at the end
			if (ele_n > size())
				std::uninitialized_default_construct_n(end(), ele_n - size());
			// or destroy elements at the end to have size() == ele_n
			else if (ele_n < size())
				_destroy_range(begin() + ele_n, end());

			// and update our size
			_size = ele_n;
		}

		/**
		 * @brief Range assign.
		 *
		 * @tparam It Iterator type.
		 *
		 * @param from Iterator to start of range.
		 * @param to Iterator to end of range.
		 */
		template <std::input_iterator It>
		constexpr void assign(It from, It to)
		{
			_assert_valid_after_clear(from, to);
			clear();
			append(from, to);
		}
		/**
		 * @brief Initializer list assign.
		 *
		 * @param il Initializer list.
		 */
		constexpr void assign(std::initializer_list<T> il)
		{
			clear();
			append(il);
		}

		/**
		 * @brief Copy assign.
		 */
		constexpr void assign(const small_vector_ref& rhs)
		{
			assign(rhs.begin(), rhs.end());
		}

		/**
		 * @note ELEMENT ACCESS
		 */
		 /**
		  * @brief Accesses specified element at index by reference with
		  * bounds checking.
		  *
		  * @throw std::out_of_range if index is out of range.
		  */
		VECL_NODISCARD constexpr reference at(size_type i)
		{
			if (i >= size())
				throw std::out_of_range("index out of range");
			return (*this)[i];
		}

		/**
		 * @brief Accesses specified element at index by const-reference with
		 * bounds checking.
		 *
		 * @throw std::out_of_range if index is out of range.
		 */
		VECL_NODISCARD constexpr const_reference at(size_type i) const
		{
			if (i >= size())
				throw std::out_of_range("index out of range");
			return (*this)[i];
		}

		/**
		 * @brief Subscript operator.
		*/
		VECL_NODISCARD constexpr reference operator[](size_type i)
		{
			VECL_ASSERT(i < size());
			return begin()[i];
		}

		/**
		 * @brief Subscript operator.
		*/
		VECL_NODISCARD constexpr const_reference operator[](size_type i) const
		{
			VECL_ASSERT(i < size());
			return begin()[i];
		}

		/**
		 * @brief Accesses first element by reference.
		 */
		VECL_NODISCARD constexpr reference front()
		{
			VECL_ASSERT(!empty());
			return begin()[0];
		}

		/**
		 * @brief Accesses first element by const-reference.
		 */
		VECL_NODISCARD constexpr const_reference front() const
		{
			VECL_ASSERT(!empty());
			return begin()[0];
		}

		/**
		 * @brief Accesses last element by reference.
		 */
		VECL_NODISCARD constexpr reference back()
		{
			VECL_ASSERT(!empty());
			return end()[-1];
		}

		/**
		 * @brief Accesses last element by const-reference.
		 */
		VECL_NODISCARD constexpr const_reference back() const
		{
			VECL_ASSERT(!empty());
			return end()[-1];
		}
		/**
		 * @brief Direct access to underlying buffer by pointer.
		 */
		VECL_NODISCARD constexpr pointer data()
		{
			return pointer(begin());
		}

		/**
		 * @brief Direct access to underlying buffery by const pointer.
		 */
		VECL_NODISCARD constexpr const_pointer data() const
		{
			return const_pointer(begin());
		}


		/**
		 * @note ITERATORS
		 */

		 /**
		  * @brief Standard Iterable Object boilerplate.
		  * @return Iterator to beginning of range.
		  */
		VECL_NODISCARD constexpr iterator begin() VECL_NOEXCEPT
		{
			return (iterator)(_buffer);
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Iterator to end of range.
		 */
		VECL_NODISCARD constexpr iterator end() VECL_NOEXCEPT
		{
			return (iterator)(_buffer)+_size;
		}
		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Const-iterator to beginning of range.
		 */
		VECL_NODISCARD constexpr const_iterator begin() const VECL_NOEXCEPT
		{
			return (const_iterator)(_buffer);
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Const-iterator to end of range.
		 */
		VECL_NODISCARD constexpr const_iterator end() const VECL_NOEXCEPT
		{
			return (const_iterator)(_buffer)+_size;
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Const-iterator to beginning of range.
		 */
		VECL_NODISCARD constexpr const_iterator cbegin() const VECL_NOEXCEPT
		{
			return begin();
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Const-iterator to end of range.
		 */
		VECL_NODISCARD constexpr const_iterator cend() const VECL_NOEXCEPT
		{
			return end();
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Reverse Iterator to beginning of range.
		 */
		VECL_NODISCARD constexpr reverse_iterator rbegin() VECL_NOEXCEPT
		{
			return std::make_reverse_iterator(end());
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Reverse Iterator to end of range.
		 */
		VECL_NODISCARD constexpr reverse_iterator rend() VECL_NOEXCEPT
		{
			return std::make_reverse_iterator(begin());
		}
		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Reverse Const-iterator to beginning of range.
		 */
		VECL_NODISCARD
			constexpr const_reverse_iterator rbegin() const VECL_NOEXCEPT
		{
			return std::make_reverse_iterator(end());
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Reverse Const-iterator to end of range.
		 */
		VECL_NODISCARD
			constexpr const_reverse_iterator rend() const VECL_NOEXCEPT
		{
			return std::make_reverse_iterator(begin());
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Reverse Const-iterator to beginning of range.
		 */
		VECL_NODISCARD
			constexpr const_reverse_iterator crbegin() const VECL_NOEXCEPT
		{
			return std::make_reverse_iterator(end());
		}

		/**
		 * @brief Standard Iterable Object boilerplate.
		 * @return Reverse Const-iterator to end of range.
		 */
		VECL_NODISCARD
			constexpr const_reverse_iterator crend() const VECL_NOEXCEPT
		{
			return std::make_reverse_iterator(begin());
		}

		/**
		 * @note CAPACITY
		 */

		 /**
		  * @brief Checks if the container is empty.
		  */
		VECL_NODISCARD constexpr bool empty() const VECL_NOEXCEPT
		{
			return _size == 0;
		}

		/**
		 * @return Size of container.
		 */
		VECL_NODISCARD constexpr size_type size() const VECL_NOEXCEPT
		{
			return _size;
		}

		/**
		 * @return Capacity of container.
		 */
		VECL_NODISCARD constexpr size_type capacity() const VECL_NOEXCEPT
		{
			return _capacity;
		}

		/**
		 * @return Max size of container.
		 */
		VECL_NODISCARD constexpr size_type max_size() const VECL_NOEXCEPT
		{
			return std::numeric_limits<size_type>::max();
		}

		constexpr void reserve(size_type N)
		{
			if (capacity() < N)
				_grow(N);
		}

		
		VECL_NODISCARD constexpr bool is_small() const 
		{ 
			return begin() == _get_first();
		}


		/**
		 * @note MODIFIERS
		 */
		 /**
		  * @brief Clears the container.
		  */
		constexpr void clear() VECL_NOEXCEPT
		{
			_destroy_range(begin(), end());
			_size = 0;
		}

		/**
		 * @brief Range append.
		 *
		 * @tparam It Iterator type.
		 *
		 * @param from Iterator to start of range.
		 * @param to Iterator to end of range.
		 */
		template <std::input_iterator It>
		constexpr void append(It from, It to)
		{
			// get the number of elements we are inserting
			size_type ele_n = std::distance(from, to);

			// if It is of type T*, we have to check if the range belongs
			// to the container
			if constexpr (std::is_same<std::remove_const_t<It>, T*>::value)
			{
				// if it does, then we have to take extra care not to
				// invalidate them
				if (_is_range_in_buffer(from, to) && _will_grow(ele_n))
				{
					// convert iterators to indexes
					size_t from_i = from - begin();
					size_t to_i = to - begin();
					
					// reserve the new size
					reserve(size() + ele_n);

					//un-invalidate iterators
					auto from_n = begin() + from_i;
					auto to_n = begin() + to_i;

					// copy from the range
					std::uninitialized_copy(from_n, to_n, end());
					_size += ele_n;
				}
			}

			// else we do not do any checking and just copy the elements over
			reserve(size() + ele_n);
			std::uninitialized_copy(from, from + ele_n, end());
			_size += ele_n;
		}

		/**
		 * @brief Fill append.
		 *
		 * @param ele_n Number of elements
		 * @param ele Element to fill by const-reference
		 */
		constexpr void append(
			size_type ele_n,
			const value_type& ele
		)
		{
			// in case ele is a ref in our container, we use _reserve_for_param
			// to ensure the ref isn't invalidated when we reserve
			const T* ele_p = _reserve_for_param(ele_n, ele);
			std::uninitialized_fill_n(end(), ele_n, *ele_p);
			_size += ele_n;
		}

		/**
		 * @brief Default Fill append.
		 *
		 * @param ele_n Number of elements
		 */
		constexpr void append(size_type ele_n)
		{
			// reserve for at least ele_n more elements
			reserve(size() + ele_n);
			std::uninitialized_default_construct_n(end(), ele_n);
			_size += ele_n;
		}

		/**
		 * @brief Initializer list append.
		 *
		 * @param il Initializer list.
		 */
		constexpr void append(std::initializer_list<T> il)
		{
			append(il.begin(), il.end());
		}

		/**
		 * @brief Inserts ele before cit in the container.
		 *
		 * @param cit Const-iterator to insert before
		 * @param ele value_type by universal-reference
		 */
		constexpr iterator insert(const_iterator cit, value_type&& ele)
		{
			// we cast away the constness of our iterator since
			// it is ours and this is a non-const function
			iterator it = const_cast<iterator>(cit);
			// if it == end(), we can just push back
			if (it == end())
			{
				push_back(std::move(ele));
				return end() - 1;
			}

			VECL_ASSERT(
				_is_reference_in_buffer(it),
				"insertion iterator is out of bounds"
			);

			// in case we grow, we have to make sure our iterator is not
			// invalidated; so we change it to an index
			size_t it_i = it - begin();
			// in case ele is a ref in our container, we use _reserve_for_param
			// to ensure the ref isn't invalidated when we reserve
			T* ele_p = _reserve_for_param(1, ele);
			// uninvalidate iterator
			it = begin() + it_i;

			// we move construct the back element to the uninitialized memory
			new ((void*)end()) T(std::move(back()));
			// then we can move assign the rest of them one step to the right
			std::move_backward(it, end() - 1, end());
			++_size;

			// if our reference was in the range of the move
			if (_is_reference_in_range(ele_p, it, end()))
				++ele_p; // we adjust the position of the ptr

			// finally, just assign the element in place
			*it = std::move(*ele_p);

			return it;
		}

		/**
		 * @brief Inserts ele before cit in the container.
		 *
		 * @param cit Const-iterator to insert before
		 * @param ele value_type by const-reference
		 */
		constexpr iterator insert(const_iterator cit, const value_type& ele)
		{
			return insert(cit, value_type(ele));
		}

		/**
		 * @brief Inserts ele_n number of elements before cit in the container.
		 *
		 * @param cit Const-iterator to insert before
		 * @param ele_n Number of elements
		 * @param ele value_type by const-reference
		 */
		constexpr iterator insert(
			const_iterator cit,
			size_type ele_n,
			const value_type& ele
		)
		{
			// we cast away the constness of our iterator since
			// it is ours and this is a non-const function
			iterator it = const_cast<iterator>(cit);
			// if it == end(), we can append
			if (it == end())
			{
				append(ele_n, ele);
				return it;
			}

			VECL_ASSERT(
				_is_reference_in_buffer(it),
				"insertion iterator is out of bounds"
			);

			// in case we grow, we have to make sure our iterator is not
			// invalidated; so we change it to an index
			size_t it_i = it - begin();
			// in case ele is a ref in our container, we use _reserve_for_param
			// to ensure the ref isn't invalidated when we reserve
			const value_type* ele_p = _reserve_for_param(ele_n, ele);
			// uninvalidate iterator
			it = begin() + it_i;

			// save the end
			value_type* old = end();
			// if overwriting elements is >= to inserting elements
			if ((size_type)std::distance(it, end()) >= ele_n)
			{
				// we have the simple case of appending the last
				// ele_n number of elements to the end
				append(std::make_move_iterator(end() - ele_n),
					std::make_move_iterator(end()));
				// and moving the rest backwards accordingly
				std::move_backward(it, old - ele_n, old);

				// if our reference was in the range of the move
				// we adjust the position of the ptr
				if (it <= ele_p && ele_p < end())
					std::advance(ele_p, ele_n); 
				// finally, just fill the elements in the new space
				std::fill_n(it, ele_n, *ele_p);
				return it;
			}
			// else we update the size here so we can use the new end()
			_size += ele_n;
			// overwrite n is the number of elements we need to move
			size_t overwrite_n = std::distance(it, old);
			
			// move last overwrite_n number of elements to uninitialized memory
			// we know it is definitely uninitialized since 
			// overwrite_n < ele_n
			std::uninitialized_move(it, old, end() - overwrite_n);

			// if our reference was in the range of the move
			// we adjust the position of the ptr
			if (it <= ele_p && ele_p < end())
				std::advance(ele_p, ele_n);

			// fill over existing elements
			// over uninitialized fill unfilled space
			std::fill_n(it, overwrite_n, *ele_p);
			std::uninitialized_fill_n(old, ele_n - overwrite_n, *ele_p);

			return it;
		}

		/**
		 * @brief Inserts range before cit in the container.
		 *
		 * @tparam It Iterator type.
		 *
		 * @param cit Const-iterator to insert before
		 * @param from Iterator to start of range.
		 * @param to Iterator to end of range.
		 *
		 * @todo optimize for grow
		 */
		template <std::input_iterator It>
		constexpr iterator insert(const_iterator cit, It from, It to)
		{
			// we cast away the constness of our iterator since
			// it is ours and this is a non-const function
			iterator it = const_cast<iterator>(cit);
			// if it == end(), we can append
			if (it == end())
			{
				append(from, to);
				return it;
			}

			VECL_ASSERT(
				_is_reference_in_buffer(it),
				"insertion iterator is out of bounds"
			);
			// calculate number of elements to insert
			size_type ele_n = std::distance(from, to);

			// in case we grow, we have to make sure our iterator is not
			// invalidated; so we change it to an index
			size_t it_i = it - begin();
			// in case ele is a ref in our container, we use _reserve_for_param
			// to ensure the ref isn't invalidated when we reserve
			const value_type* ele_p = _reserve_for_param(ele_n, *from);
			// uninvalidate iterator
			it = begin() + it_i;

			// save the end
			value_type* old = end();
			size_t overwrite_n = (size_type)std::distance(it, old);

			// if It is of type T*, we have to check if the it is a reference
			// to the range
			if constexpr (std::is_same<std::remove_const_t<It>, T*>::value)
			{
				// if it does, then we have to take extra care when copying
				// since our range will be invalidated by the insert
				if (_is_reference_in_range(it, from, to))
				{
					// if overwriting elements is >= to inserting elements
					if (overwrite_n >= ele_n)
					{
						// we have the simple case of appending the last
						// ele_n number of elements to the end
						append(std::make_move_iterator(end() - ele_n),
							std::make_move_iterator(end()));

						// and moving the rest backwards accordingly
						value_type* last = std::move_backward(
							it, old - ele_n, old);
						
						// then we piece wise copy from both parts of the range
						// into the freed up space in the middle
						value_type* mid = std::copy(&*from, it, it);
						std::copy(last, &*to + ele_n, mid);
						return it;
					}
					// else we update the size here so we can use the new end()
					_size += ele_n;
					value_type* last = end() - overwrite_n;
					
					// move last overwrite_n number of elements to unintialized 
					// memory; we know it is definitely uninitialized since 
					// overwrite_n < ele_n
					std::uninitialized_move(it, old, end() - overwrite_n);

					// piecewise copy and move from both parts of the range
					// the freed up space in the middle is only partially 
					// filled
					value_type* curr = it;
					for (; overwrite_n > 0; --overwrite_n, ++curr, ++from)
					{
						// if we reach the start of the freed space
						// then the next element in range must be
						// ele_n away
						if (from == it)
							std::advance(from, ele_n);
						*curr = *from;
					}

					for (; curr != last; ++curr, ++from)
					{
						// if we reach the start of the freed space
						// then the next element in range must be
						// ele_n away
						if (from == it)
							std::advance(from, ele_n);
						new ((void*)curr) value_type(*from);
					}

					return it;
				}
			}

			// if overwriting elements is >= to inserting elements
			if (overwrite_n >= ele_n)
			{
				// we have the simple case of appending the last
				// ele_n number of elements to the end
				append(std::make_move_iterator(end() - ele_n),
					std::make_move_iterator(end()));
				// and moving the rest backwards accordingly
				std::move_backward(it, old - ele_n, old);
				// here we are guaranteed that the iterator is not in the range
				// but we still have to check if the range will be affected
				// by the move; if it is, adjust the positions of the range
				if (it <= ele_p && ele_p < end())
				{
					std::advance(from, ele_n);
					std::advance(to, ele_n);
				}
				// then we can trivially copy them to the space in the
				// middle
				std::copy(from, to, it);
				return it;
			}

			// else we update the size here so we can use the new end()
			_size += ele_n;
			// move last overwrite_n number of elements to uninitialized memory
			// we know it is definitely uninitialized since 
			// overwrite_n < ele_n
			std::uninitialized_move(it, old, end() - overwrite_n);
			// we do not need to check if our reference is in the range since
			// by this point, it is impossible to be
			// so we can just copy over the existing elements
			for (
				value_type* curr = it;
				overwrite_n > 0;
				--overwrite_n, ++curr, ++from
				)
				*curr = *from;
			// and construct at the initialized memory
			std::uninitialized_copy(from, to, old);

			return it;
		}

		/**
		 * @brief Inserts initializer list before cit in the container.
		 *
		 * @param cit Const-iterator to insert before
		 * @param il Initializer list.
		 */
		constexpr iterator insert(
			const_iterator cit,
			std::initializer_list<T> il)
		{
			return insert(cit, il.begin(), il.end());
		}

		/**
		 * @brief In-place constructs value_type with args before cit in
		 * the container.
		 *
		 * @param cit Const-iterator to insert before
		 * @param args Variadic arguments
		 */
		template<typename... Args>
		constexpr iterator emplace(const_iterator cit, Args&&... args)
		{
			return insert(cit, value_type(std::forward<Args>(args)...));
		}

		/**
		 * @brief Erases the element at cit from the container.
		 *
		 * @param cit Const-iterator position
		 */
		constexpr iterator erase(const_iterator cit)
		{
			VECL_ASSERT(
				_is_reference_in_buffer(cit),
				"erase iterator is out of bounds"
			);

			// we cast away the constness of our iterator since
			// it is ours and this is a non-const function
			iterator it = const_cast<iterator>(cit);
			// just move one step to the left and pop the last
			std::move(it + 1, end(), it);
			pop_back();

			return it;
		}

		/**
		 * @brief Erases a range from the container.
		 *
		 * @param cfrom Const-iterator to start of range.
		 * @param cto Const-iterator to end of range.
		 */
		constexpr iterator erase(const_iterator cfrom, const_iterator cto)
		{
			VECL_ASSERT(
				_is_range_in_buffer(cfrom, cto),
				"erase iterator range is out of bounds"
			);

			// we cast away the constness of our range iterators since
			// they are ours and this is a non-const function
			iterator from = const_cast<iterator>(cfrom);
			iterator to = const_cast<iterator>(cto);
			// just move left and destroy
			iterator last = std::move(to, end(), from);
			_destroy_range(last, end());
			_size -= std::distance(from, to);

			return from;
		}

		/**
		 * @brief Appends value to the end of the container.
		 *
		 * @param ele value_type by universal-reference
		 */
		constexpr void push_back(value_type&& ele)
		{
			// in case ele is a ref in our container, we use _reserve_for_param
			// to ensure the ref isn't invalidated when we reserve
			T* ele_p = _reserve_for_param(1, ele);
			new ((void*)end()) value_type(std::move(*ele_p));
			++_size;
		}

		/**
		 * @brief Appends value to the end of the container.
		 *
		 * @param ele value_type by const-reference
		 */
		constexpr void push_back(const value_type& ele)
		{
			// in case ele is a ref in our container, we use _reserve_for_param
			// to ensure the ref isn't invalidated when we reserve
			const T* ele_p = _reserve_for_param(1, ele);
			new ((void*)end()) value_type(*ele_p);
			++_size;
		}

		/**
		 * @brief In-place constructs value_type with args at the end of
		 * the container.
		 *
		 * @param args Variadic arguments
		 */
		template<typename... Args>
		constexpr reference emplace_back(Args&&... args)
		{
			// if we need to grow, use helper which ensures our args are not
			// invalidated by the grow
			if (size() >= capacity())
				return _grow_and_emplace_back(std::forward<Args>(args)...);

			new ((void*)end()) value_type(std::forward<Args>(args)...);
			++_size;
			return back();
		}

		/**
		 * @brief Removes the last element in the container.
		 */
		constexpr void pop_back()
		{
			VECL_ASSERT(!empty());
			--_size;
			end()->~T();
		}


		/**
		 * @brief Swaps the contents of two small_vectors.
		 */
		constexpr void swap(small_vector_ref& other)
		{
			if (&other == this) return;

			// swap the buffers if neither is small
			if (!is_small() && !other.is_small())
			{
				std::swap(_buffer, other._buffer);
				std::swap(_size, other._size);
				std::swap(_capacity, other._capacity);
				return;
			}
			// make sure both vectors have enough space
			reserve(other.size());
			other.reserve(size());

			// get the shared number of elements
			size_type shared_n = std::min(size(), other.size());
			// and swap only up to the shared number
			for (size_type i = 0; i < shared_n; ++i)
				std::swap(begin()[i], other.begin()[i]);

			// then depending on which is vector is bigger
			// we move the extra elements from the bigger vector to the
			// smaller vector, then update the size and capacity
			if (size() > other.size())
			{
				size_type diff = size() - other.size();
				std::uninitialized_move(begin() + shared_n, end(), other.end());
				other._size += diff;

				_destroy_range(begin() + shared_n, end());
				_size = shared_n;
			}
			else if (size() < other.size())
			{
				size_type diff = other.size() - size();
				std::uninitialized_move(other.begin() + shared_n, other.end(), end());
				_size += diff;

				_destroy_range(other.begin() + shared_n, other.end());
				other._size = shared_n;
			}
		}

		/**
		 * @brief Swaps the contents of two fixed_vectors.
		 */
		friend inline void swap(
			small_vector_ref& lhs,
			small_vector_ref& rhs
		) VECL_NOEXCEPT
		{
			lhs.swap(rhs);
		}

	protected:
		void* _buffer;
		size_type _size, _capacity;
	};

	/**
	 * @brief A Small Vector is a dynamic array with small buffer
	 *  optimizations; i.e. for small sizes, the elements are stored
	 *  on the stack instead of the heap. Only
	 *  when the stack buffer is full do we then 'spill' on to the heap.
	 * 	 *
	 * @tparam T Type of element.
	 * @tparam N Size of the small buffer.
	 */
	template<typename T, size_t N>
	class small_vector : public small_vector_ref<T>, small_vector_buffer<T, N>
	{
		using super = small_vector_ref<T>;
	public:
		/**
		 * @brief Default Constructor.
		 */
		small_vector() : super(N) {}

		/**
		* @brief Destructor.
		*/
		constexpr ~small_vector()
		{
			super::clear();
		}

		/**
		 * @brief Explicit Fill Constructor.
		 *
		 * @param ele_n Number of elements
		 * @param ele Element to fill by const-reference
		 */
		constexpr explicit small_vector(
			size_t ele_n,
			const T& ele
		) : 
			super(N)
		{
			super::append(ele_n, ele);
		}

		/**
		 * @brief Explicit Fill Constructor.
		 *
		 * @param ele_n Number of elements
		 */
		constexpr explicit small_vector(size_t ele_n) :	super(N)
		{
			super::append(ele_n);
		}

		/**
		 * @brief Range Constructor.
		 *
		 * @tparam It Iterator type.
		 *
		 * @param from Iterator to start of range.
		 * @param to Iterator to end of range.
		 */
		template <std::input_iterator It>
		constexpr explicit small_vector(It from, It to)
			: super(N)
		{
			super::append(from, to);
		}

		/**
		 * @brief Copy Constructor.
		 */
		constexpr small_vector(const small_vector& other)
			: super(N)
		{
			if (!other.empty())
				super::operator=(other);
		}

		/**
		 * @brief Move Constructor.
		 */
		constexpr small_vector(small_vector&& other) VECL_NOEXCEPT_IF(false)
			: super(N)
		{
			if (!other.empty())
				super::operator=(std::move(other));
		}

		/**
		 * @brief Ref Copy Constructor.
		 */
		constexpr small_vector(const small_vector_ref<T>& other)
			: super(N)
		{
			if (!other.empty())
				super::operator=(other);
		}

		/**
		 * @brief Ref Move Constructor.
		 */
		constexpr small_vector(
			small_vector_ref<T>&& other
		) VECL_NOEXCEPT_IF(false)
			: super(N)
		{
			if (!other.empty())
				super::operator=(std::move(other));
		}

		/**
		 * @brief Initializer list Constructor.
		 *
		 * @param il Initializer list.
		 */
		constexpr small_vector(std::initializer_list<T> il)
			: super(N)
		{
			super::assign(il);
		}

		/**
		 * @brief Copy-Assignment Operator.
		 */
		constexpr small_vector& operator=(const small_vector& other)
		{
			super::operator=(other);
			return *this;
		}

		/**
		 * @brief Move-Assignment Operator.
		 */
		constexpr small_vector& operator=(
			small_vector&& other
		) VECL_NOEXCEPT_IF(false)
		{
			super::operator=(std::move(other));
			return *this;
		}

		/**
		 * @brief Ref Copy-Assignment Operator.
		 */
		constexpr small_vector& operator=(const small_vector_ref<T>& other)
		{
			super::operator=(other);
			return *this;
		}

		/**
		 * @brief Ref Move-Assignment Operator.
		 */
		constexpr small_vector& operator=(
			small_vector_ref<T>&& other
		) VECL_NOEXCEPT_IF(false)
		{
			super::operator=(std::move(other));
			return *this;
		}

		/**
		 * @brief IL-Assignment Operator.
		 *
		 * @param il Initializer list.
		 */
		constexpr small_vector& operator=(std::initializer_list<T> il)
		{
			super::assign(il);
			return *this;
		}
	};
}


namespace std
{
	/**
	 * @brief Swaps the contents of two small_vectors.
	 */
	template<typename T, size_t N>
	inline void swap(
		vecl::small_vector<T, N>& lhs,
		vecl::small_vector<T, N>& rhs
	) noexcept
	{
		lhs.swap(rhs);
	}

	/**
	 * @brief Swaps the contents of two small_vectors.
	 */
	template<typename T, size_t N>
	inline void swap(
		vecl::small_vector_ref<T>& lhs,
		vecl::small_vector_ref<T>& rhs
	) noexcept
	{
		lhs.swap(rhs);
	}
}

#endif
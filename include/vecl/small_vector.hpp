#ifndef VECL_SMALL_VECTOR_H
#define VECL_SMALL_VECTOR_H

#include "config/config.h"

#include <vector>

namespace vecl
{
	template<typename T, size_t N>
	struct small_vector_buffer
	{
		alignas(T) char buffer[N * sizeof(T)];
	};

	template <typename T> 
	struct alignas(T) small_vector_buffer<T, 0> {};


	template <typename T>
	class small_vector_i : public std::vector<T>
	{

	};

	template<typename T, size_t N>
	class small_vector : public small_vector_i<T>, small_vector_buffer<T,N>
	{
	public:
		small_vector() : small_vector_i<T>(N) {}

        explicit small_vector(size_t size, const T& value = T())
            : small_vector_i<T>(N) {
        }

	};
}
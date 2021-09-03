#include <gtest/gtest.h>
#include <vecl/memory.hpp>
#include <vecl/sparse_set.hpp>

TEST(MEMORY_DEBUG, test_vector_memory) {

	{
		vecl::memory_debug a(
			"name",
			vecl::get_default_memory_debug_callback());

		std::pmr::vector<int>b(&a);
		b.reserve(100);
		int t = 100;
		while (t--)
			b.push_back(t);
	}
}
TEST(MEMORY_DEBUG, test_sparse_memory) {

	{
		vecl::memory_debug a(
			"name",
			vecl::get_default_memory_debug_callback());

		vecl::sparse_set b{ { 1,2,3,4,5 }, &a };
	}
	{
		vecl::memory_debug a(
			"aligned alloc",
			vecl::get_default_memory_debug_callback(),
			vecl::get_memory_aligned_alloc());
	}

}


TEST(MEMORY_DEBUG, reg_callback) {
	// overaligned
	{
		vecl::memory_debug a(
			"name",
			vecl::get_default_memory_debug_callback());

		auto p = a.allocate(10, 16);
		a.deallocate(p, 10, 16);
	}
	// leak
	{
		vecl::memory_debug a(
			"name",
			vecl::get_default_memory_debug_callback());

		auto p = a.allocate(10, 8);
	}
	// double delete
	{
		vecl::memory_debug a(
			"name",
			vecl::get_default_memory_debug_callback());

		auto p = a.allocate(10, 8);
		a.deallocate(p, 10, 8);
		a.deallocate(p, 10, 8);
	}
	// invalid delete
	{
		vecl::memory_debug a(
			"name",
			vecl::get_default_memory_debug_callback());

		int b, * c = &b;
		a.deallocate(c, 8, 8);
	}
	// over run
	{
		vecl::memory_debug a(
			"name",
			vecl::get_default_memory_debug_callback());

		auto p = (char*)a.allocate(4, 4);
		p[4] = 'a';
		a.deallocate(p, 4, 4);
	}
	// over run bad string
	{
		class lousy_string
		{
			using allocator_type = std::pmr::polymorphic_allocator<std::byte>;

			allocator_type m_allocator;
			size_t m_length;
			char* m_buffer;

		public:
			lousy_string(const char* c_str, allocator_type allocator)
				: m_allocator{ allocator },
				m_length{ std::strlen(c_str) },
				m_buffer{ (char*)m_allocator.resource()->allocate(m_length, 1) }
				// intentionally allocating one less byte for null terminator
			{
				std::strcpy(m_buffer, c_str);
			}

			~lousy_string()
			{
				m_allocator.resource()->deallocate(m_buffer, m_length, 1);
			}
		};

		vecl::memory_debug a(
			"name",
			vecl::get_default_memory_debug_callback());

		lousy_string b("testing", &a);

	}
	// under run
	{
		vecl::memory_debug a(
			"name",
			vecl::get_default_memory_debug_callback());

		auto p = (char*)a.allocate(4, 4);
		p[-1] = 'a';
		a.deallocate(p, 4, 4);
	}
	// bad alloc
	{
		try
		{
			vecl::memory_debug a(
				"name",
				vecl::get_default_memory_debug_callback());

			auto p = (char*)a.allocate(1000000000000000000, 4);
			a.deallocate(p, 1000000000000000000, 4);
		}
		catch (...)
		{

		}
	}
	// null delete
	{
		vecl::memory_debug a(
			"name",
			vecl::get_default_memory_debug_callback());

		a.deallocate(nullptr, 0, 4);
	}

}

TEST(MEMORY_DEBUG, mt_callback) {
	// overaligned
	{
		vecl::memory_debug_mt a(
			"name",
			vecl::get_default_memory_debug_callback());

		auto p = a.allocate(10, 16);
		a.deallocate(p, 10, 16);
	}
	// leak
	{
		vecl::memory_debug_mt a(
			"name",
			vecl::get_default_memory_debug_callback());

		auto p = a.allocate(10, 8);
	}
	// double delete
	{
		vecl::memory_debug_mt a(
			"name",
			vecl::get_default_memory_debug_callback());

		auto p = a.allocate(10, 8);
		a.deallocate(p, 10, 8);
		a.deallocate(p, 10, 8);
	}
	// invalid delete
	{
		vecl::memory_debug_mt a(
			"name",
			vecl::get_default_memory_debug_callback());

		int b, * c = &b;
		a.deallocate(c, 8, 8);
	}
	// over run
	{
		vecl::memory_debug_mt a(
			"name",
			vecl::get_default_memory_debug_callback());

		auto p = (char*)a.allocate(4, 4);
		p[4] = 'a';
		a.deallocate(p, 4, 4);
	}
	// over run bad string
	{
		class lousy_string
		{
			using allocator_type = std::pmr::polymorphic_allocator<std::byte>;

			allocator_type m_allocator;
			size_t m_length;
			char* m_buffer;

		public:
			lousy_string(const char* c_str, allocator_type allocator)
				: m_allocator{ allocator },
				m_length{ std::strlen(c_str) },
				m_buffer{ (char*)m_allocator.resource()->allocate(m_length, 1) }
				// intentionally allocating one less byte for null terminator
			{
				std::strcpy(m_buffer, c_str);
			}

			~lousy_string()
			{
				m_allocator.resource()->deallocate(m_buffer, m_length, 1);
			}
		};

		vecl::memory_debug_mt a(
			"name",
			vecl::get_default_memory_debug_callback());

		lousy_string b("testing", &a);

	}
	// under run
	{
		vecl::memory_debug_mt a(
			"name",
			vecl::get_default_memory_debug_callback());

		auto p = (char*)a.allocate(4, 4);
		p[-1] = 'a';
		a.deallocate(p, 4, 4);
	}
	// bad alloc
	{
		try
		{
			vecl::memory_debug_mt a(
				"name",
				vecl::get_default_memory_debug_callback());

			auto p = (char*)a.allocate(1000000000000000000, 4);
			a.deallocate(p, 1000000000000000000, 4);
		}
		catch (...)
		{

		}
	}
	// null delete
	{
		vecl::memory_debug_mt a(
			"name",
			vecl::get_default_memory_debug_callback());

		a.deallocate(nullptr, 0, 4);
	}


}
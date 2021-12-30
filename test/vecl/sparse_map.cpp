
#include <vecl/sparse_map.hpp>
#include <vecl/memory.hpp>
#include <gtest/gtest.h>
using sparse = vecl::sparse_map<uint32_t, std::string>;

TEST(SPARSE_MAP, default_constructor) {
	sparse a;
	ASSERT_EQ(a.count(1), 0);
	ASSERT_EQ(a.size(), 0);
}
TEST(SPARSE_MAP, size_constructor) {
	sparse a(1000);
	ASSERT_EQ(a.max_size(), 1000);
	ASSERT_EQ(a.max(), 999);
}
TEST(SPARSE_MAP, range_constructor) {
	std::unordered_map< uint32_t, std::string> a =
	{
		{1, "string"}, {2, "string"}, {3, "string"}
	};

	sparse b(a.begin(), a.end());
	ASSERT_EQ(b.count(1), 1);
	ASSERT_EQ(b.count(2), 1);
	ASSERT_EQ(b.count(3), 1);
	ASSERT_EQ(b[1], "string");

	//auto& [id, str] = *a.begin();
}
TEST(SPARSE_MAP, il_constructor) {

	sparse a =
	{
		{1, "string"}, {2, "string"}, {3, "string"}
	};

	ASSERT_EQ(a.count(1), 1);
	ASSERT_EQ(a.count(2), 1);
	ASSERT_EQ(a.count(3), 1);
	ASSERT_EQ(a[1], "string");
}
TEST(SPARSE_MAP, copy_constructor) {

	sparse b =
	{
		{1, "string"}, {2, "string"}, {3, "string"}
	};
	
	sparse a{ b , std::pmr::get_default_resource()};
	ASSERT_EQ(a.count(1), 1);
	ASSERT_EQ(a.count(2), 1);
	ASSERT_EQ(a.count(3), 1);
	ASSERT_EQ(a.count(4), 0);
	ASSERT_EQ(a[1], "string");
}
TEST(SPARSE_MAP, move_constructor) {

	sparse b =
	{
		{1, "string"}, {2, "string"}, {3, "string"}
	};

	sparse a{ std::move(b)};
	ASSERT_EQ(a.count(1), 1);
	ASSERT_EQ(a.count(2), 1);
	ASSERT_EQ(a.count(3), 1);
	ASSERT_EQ(b.size(), 0);
	ASSERT_EQ(a[1], "string");
}
TEST(SPARSE_MAP, move_assign_lol) {

	sparse b =
	{
		{1, "string"}, {2, "string"}, {3, "string"}
	};

	sparse a{ vecl::get_memory_malloc() };

	a = std::move(b);

	ASSERT_EQ(a.count(1), 1);
	ASSERT_EQ(a.count(2), 1);
	ASSERT_EQ(a.count(3), 1);
	ASSERT_EQ(b.size(), 0);
	ASSERT_EQ(a[1], "string");
}
TEST(SPARSE_MAP, copy_assign) {

	sparse b =
	{
		{1, "string"}, {2, "string"}, {3, "string"}
	};

	sparse a{ {4, ""} };
	a = b;

	ASSERT_EQ(a.count(1), 1);
	ASSERT_EQ(a.count(2), 1);
	ASSERT_EQ(a.count(3), 1);
	ASSERT_EQ(b.count(1), 1);
	ASSERT_EQ(a[1], "string");
}
TEST(SPARSE_MAP, move_assign) {

	sparse b =
	{
		{1, "string"}, {2, "string"}, {3, "string"}
	};

	sparse a{ {4, ""} };
	a = std::move(b);

	ASSERT_EQ(a.count(1), 1);
	ASSERT_EQ(a.count(2), 1);
	ASSERT_EQ(a.count(3), 1);
	ASSERT_EQ(a.count(4), 0);
	ASSERT_EQ(b.count(1), 0);
	ASSERT_EQ(a[1], "string");
}
TEST(SPARSE_MAP, front_back) {

	sparse a =
	{
		{1, "string"}, {2, "string"}, {3, "string"}
	};

	auto [id1, t1] = a.front();
	auto [id2, t2] = a.back();

	ASSERT_EQ(t1, a[id1]);
	ASSERT_EQ(t1, "string");
	ASSERT_EQ(t2, a[id2]);
	ASSERT_EQ(t2, "string");

	t1 = "new";
}
TEST(SPARSE_MAP, for_loop) {

	sparse a =
	{
		{1, "string"}, {2, "string"}, {3, "string"}
	};

	for (auto [i, str] : a)
	{
		str = "yo";
	}

	ASSERT_EQ(a[1], "yo");
	ASSERT_EQ(a.begin()->second, "yo");
}
TEST(SPARSE_MAP, emplace_push_insert) {

	sparse a;
	{
		auto [it, b] = a.emplace_back(66, "string");
		ASSERT_EQ(b, true);
		ASSERT_EQ(it->second, "string");
		ASSERT_EQ(a[66], "string");
	}
	{
		auto [it, b] = a.emplace_back(66, "string");
		ASSERT_EQ(b, false);
		ASSERT_EQ(it->second, "string");
		ASSERT_EQ(a[66], "string");
	}
	{
		auto [it, b] = a.emplace_back({ 2, "string" });
		ASSERT_EQ(it->second, "string");
		ASSERT_EQ(a[2], "string");
	}
	{
		auto pair = std::make_pair(123, "string");
		auto [it, b] = a.emplace_back(pair);
		ASSERT_EQ(it->second, "string");
		ASSERT_EQ(a[123], "string");
	}
	{
		auto pair = std::make_pair(54, "string");
		auto [it, b] = a.emplace_back(std::move(pair));
		ASSERT_EQ(it->second, "string");
		ASSERT_EQ(a[54], "string");
	}
	{
		a[29] = "Hello";
		ASSERT_EQ(a[29], "Hello");
	}
	{
		a[29] = "string";
		ASSERT_EQ(a[29], "string");
	}
	{
		auto pair = std::make_pair(123, "string");
		a.push_back(pair);
		ASSERT_EQ(a[123], "string");
	}
	{
		auto pair = std::make_pair(54, "string");
		a.push_back(std::move(pair));
		ASSERT_EQ(a[54], "string");
	}
	{
		a.push_back(555, "string");
		ASSERT_EQ(a[555], "string");
	}
	{
		std::string str = "string";
		a.push_back(649, std::move(str));
		ASSERT_EQ(a[649], "string");
		ASSERT_EQ(str.empty(), true);
	}
	{
		auto key = a.key_at(a.size()-1);
		a.pop_back();
		ASSERT_NE(key, a.back().first);

		while (!a.empty())
			a.pop_back();

		ASSERT_EQ(a.empty(), true);
	}
	{
		auto pair = std::make_pair(767, "string");
		auto [it, b] = a.insert(pair);
		ASSERT_EQ(it->second, "string");
		ASSERT_EQ(a[767], "string");
	}
	{
		auto pair = std::make_pair(295, "string");
		auto [it, b] = a.insert(std::move(pair));
		ASSERT_EQ(it->second, "string");
		ASSERT_EQ(a[295], "string");
	}

}

TEST(SPARSE_MAP, erase_remove) {

	sparse a;
	{
		a.clear();
		auto pair = std::make_pair(867, "string");
		auto [it, b] = a.insert(pair);
		ASSERT_EQ(a.count(867), 1);
		auto it2 = a.erase(it);
		ASSERT_EQ(it2, a.end());
		ASSERT_EQ(a.count(867), 0);
	}
	{
		auto pair = std::make_pair(4, "string");
		a.insert(std::move(pair));

		ASSERT_EQ(a.count(4), 1);
		a.erase(4);
		ASSERT_EQ(a.count(4), 0);
		ASSERT_EQ(a.erase(0), a.end());
	}
	{

		ASSERT_EQ(a.remove(0), false);
		a.push_back(10, "");
		ASSERT_EQ(a.remove(10), true);
	}

}
TEST(SPARSE_MAP, iterators) {

	sparse a =
	{
		{1, "1"}, {2, "2"}, {3, "3"}
	};

	auto b = a.begin();
	auto e = a.end();

	ASSERT_LT(b, e);
	ASSERT_GT(e, b);
	ASSERT_LE(e, e);
	ASSERT_GE(b, b);

	ASSERT_LE(b, e);
	ASSERT_LE(b + 1, e);
	ASSERT_LE(b + 2, e);
	ASSERT_LE(b + 3, e);
	ASSERT_EQ(e - b, 3);


	ASSERT_EQ(b->second, "1");
	++b;
	ASSERT_EQ(b->second, "2");
	++b;
	ASSERT_EQ(b->second, "3");
	++b;
	ASSERT_EQ(b, e);


}
TEST(SPARSE_MAP, sort) {

	sparse a =
	{
		{1, "1"}, {2, "2"}, {3, "3"}
	};
	sparse b =
	{
		{3, "3"}, {1, "1"}, {2, "2"},
	};

	ASSERT_NE(b, a);
	b.sort();
	ASSERT_EQ(a, b);
}

TEST(SPARSE_MAP, swap) {

	sparse a =
	{
		{1, "1"}, {2, "2"}, {3, "3"}
	};
	sparse b =
	{
		{3, "3"}, {1, "1"}, {2, "2"},
	};

	auto c = a;
	auto d = b;

	ASSERT_NE(c, b);
	ASSERT_NE(d, a);

	std::swap(a, b);

	ASSERT_EQ(c, b);
	ASSERT_EQ(d, a);
}



#include <gtest/gtest.h>
#include <vecl/sparse_set.hpp>

TEST(SPARSE_SET, constructor) {
	vecl::sparse_set<uint32_t, 10> a;
	vecl::sparse_set<uint64_t, 20> b;
	vecl::sparse_set<size_t, 30> c;

	ASSERT_EQ(a.size(), 0);
	ASSERT_EQ(a.max_size(), 10);
	ASSERT_EQ(b.dense_size(), 0);
	ASSERT_EQ(b.sparse_size(), 20);
	ASSERT_EQ(c.size_value, 30);
}

TEST(SPARSE_SET, emplace_push_pop) {
	vecl::sparse_set a;

	ASSERT_EQ(a.empty(), true);

	auto it = a.emplace_back(0);
	ASSERT_EQ(*it, 0);
	ASSERT_EQ(a.size(), 1);
	ASSERT_EQ(a.count(0), 1);

	a.pop_back();
	ASSERT_EQ(a.size(), 0);
	ASSERT_EQ(a.count(0), 0);
	
	a.push_back(0);
	ASSERT_EQ(a.size(), 1);
	ASSERT_EQ(a.count(0), 1);

}

TEST(SPARSE_SET, insert) {
	vecl::sparse_set a;
	{
		auto [it, b] = a.insert(0);
		ASSERT_EQ(*it, 0);
		ASSERT_EQ(b, true);
		ASSERT_EQ(a.size(), 1);
		ASSERT_EQ(a.count(0), 1);
	}
	{
		auto [it, b] = a.insert(0);
		ASSERT_EQ(*it, 0);
		ASSERT_EQ(b, false);
		ASSERT_EQ(a.size(), 1);
		ASSERT_EQ(a.count(0), 1);
	}
	{
		auto [it, b] = a.insert(1);
		ASSERT_EQ(*it, 1);
		ASSERT_EQ(b, true);
		ASSERT_EQ(a.size(), 2);
		ASSERT_EQ(a.count(1), 1);
	}
}

TEST(SPARSE_SET, find) {
	vecl::sparse_set a;
	{
		auto it = a.find(2);
		ASSERT_EQ(it, a.end());
	}
	a.emplace_back(0);
	a.emplace_back(1);
	a.emplace_back(2);
	{
		auto it = a.find(2);
		ASSERT_NE(it, a.end());
		ASSERT_EQ(*it, 2);
	}
}

TEST(SPARSE_SET, erase) {
	vecl::sparse_set a;
	a.emplace_back(0);
	a.emplace_back(1);
	{
		auto it = a.erase(2);
		ASSERT_EQ(it, a.end());
	}
	{
		auto it = a.erase(1);
		ASSERT_NE(it, a.end());
		ASSERT_EQ(*it, 0);
	}
	{
		auto it = a.erase(0);
		ASSERT_EQ(it, a.end());
	}
	a.emplace_back(0);
	a.emplace_back(1);
	{
		auto found = a.find(0);
		ASSERT_NE(found, a.end());
		auto it = a.erase(found);
		ASSERT_NE(it, a.end());
		ASSERT_EQ(*it, 1);
	}
}

TEST(SPARSE_SET, swap) {
	vecl::sparse_set a;
	auto ait = a.emplace_back(0);
	vecl::sparse_set b;
	auto bit = b.emplace_back(1);

	std::swap(a, b);
	ASSERT_EQ(*ait, 0);
	ASSERT_EQ(*bit, 1);
	ASSERT_EQ(a.count(1), 1);
	ASSERT_EQ(b.count(0), 1);
}
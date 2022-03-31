
#include <vecl/set_vector.hpp>
#include <gtest/gtest.h>
TEST(SET_VECTOR, constructor) {
	vecl::set_vector<uint32_t> a;
	ASSERT_EQ(a.size(), 0);
}


TEST(SET_VECTOR, range_constructor) {
	std::vector<float> a;
	int t = 50;
	while (t--)
		a.push_back(static_cast<float>(t));

	vecl::set_vector<size_t> b(a.begin(), a.end());

	ASSERT_EQ(b.size(), a.size());
	for (auto i : a)
		ASSERT_EQ(b.count(static_cast<uint32_t>(i)), 1);

}

TEST(SET_VECTOR, il_constructor) {
	vecl::set_vector<size_t> b{ 1,2,3,4 };

	ASSERT_TRUE(b.count(1));
	ASSERT_TRUE(b.count(2));
	ASSERT_TRUE(b.count(3));
	ASSERT_TRUE(b.count(4));
}

TEST(SET_VECTOR, emplace_push_pop) {
	vecl::set_vector<int> a;

	ASSERT_EQ(a.empty(), true);

	auto b = a.emplace_back(0);
	ASSERT_EQ(b, true);
	ASSERT_EQ(a.size(), 1);
	ASSERT_EQ(a.count(0), 1);

	a.pop_back();
	ASSERT_EQ(a.size(), 0);
	ASSERT_EQ(a.count(0), 0);

	a.push_back(0);
	ASSERT_EQ(a.size(), 1);
	ASSERT_EQ(a.count(0), 1);

}

TEST(SET_VECTOR, insert) {
	vecl::set_vector<int> a;
	{
		auto b = a.insert(0);
		ASSERT_EQ(b, true);
		ASSERT_EQ(a.size(), 1);
		ASSERT_EQ(a.count(0), 1);
	}
	{
		auto b = a.insert(0);
		ASSERT_EQ(b, false);
		ASSERT_EQ(a.size(), 1);
		ASSERT_EQ(a.count(0), 1);
	}
	{
		auto b = a.insert(1);
		ASSERT_EQ(b, true);
		ASSERT_EQ(a.size(), 2);
		ASSERT_EQ(a.count(1), 1);
	}
}



TEST(SET_VECTOR, find) {
	vecl::set_vector<int> a;
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

TEST(SET_VECTOR, erase) {
	vecl::set_vector<int> a;
	a.emplace_back(0);
	a.emplace_back(1);
	{
		auto it = a.erase(2);
		ASSERT_EQ(it, a.end());
	}
	{
		a.erase(1);
		ASSERT_TRUE(!a.contains(1));
	}
	{
		a.erase(0);
		ASSERT_TRUE(!a.contains(0));
	}
	a.emplace_back(0);
	a.emplace_back(1);
	{
		auto found = a.find(0);
		ASSERT_NE(found, a.end());
		a.erase(found);
		ASSERT_TRUE(!a.contains(0));
	}
}

TEST(SET_VECTOR, swap) {
	vecl::set_vector<int> a;
	a.emplace_back(0);
	vecl::set_vector<int> b;
	b.emplace_back(1);

	std::swap(a, b);
	ASSERT_EQ(a.count(1), 1);
	ASSERT_EQ(b.count(0), 1);

	swap(a, b);
	ASSERT_EQ(b.count(1), 1);
	ASSERT_EQ(a.count(0), 1);

	a.swap(b);
	ASSERT_EQ(a.count(1), 1);
	ASSERT_EQ(b.count(0), 1);

	b.swap(a);
	ASSERT_EQ(b.count(1), 1);
	ASSERT_EQ(a.count(0), 1);
}

TEST(SET_VECTOR, clear) {
	vecl::set_vector<int> a;
	int t = 1000;
	while (--t)
		a.push_back(t);
	a.clear();

	ASSERT_EQ(a.count(0), 0);
	ASSERT_EQ(a.count(1), 0);
	ASSERT_EQ(a.count(999), 0);
	ASSERT_EQ(a.count(1000), 0);
}


TEST(SET_VECTOR, take_vector) {

	auto equal = [](auto& a, auto& b)
	{
		if (a.size() != b.size()) return false;

		for (int i = 0; i < (int)a.size(); ++i)
		{
			if (a[i] != b[i]) return false;
		}

		return true;
	};

	std::vector<int> ref = { 1,2,3,4 };

	vecl::set_vector<int> a{ ref.begin(), ref.end() };
	auto v = a.take_vector();

	ASSERT_TRUE(a.empty());
	ASSERT_TRUE(v.size() == ref.size());
	ASSERT_TRUE(equal(v,ref));
}
/*
TEST(SET_VECTOR, equal) {
	vecl::set_vector<int> a;
	vecl::set_vector<int> b;
	vecl::set_vector<size_t> c;
	int t = 500;
	while (t--)
	{
		a.push_back(t);
		b.push_back(t);
		c.push_back(t);
	}
	ASSERT_EQ(a, b);
	ASSERT_EQ(a.set_equal(b), true);
	ASSERT_EQ(vecl::set_equal(c, b), true);
	ASSERT_EQ(vecl::set_equal(a, c, b), true);
}

TEST(SET_VECTOR, sort) {
	vecl::set_vector<int> a = { 1,2,3,4,5 };
	vecl::set_vector<int> b = { 5,4,1,2,3 };

	b.sort();

	ASSERT_EQ(a, b);
}

TEST(SET_VECTOR, merge) {
	vecl::set_vector<int> a = { 1,2,3,4,5 };
	vecl::set_vector<int> b = { 6,7,8,2,5 };
	vecl::set_vector<int> c = { 1,2,3,4,5,6,7,8 };

	a.merge(b);
	EXPECT_EQ(a, c);
	ASSERT_EQ(a.set_equal(c), true);
}

TEST(SET_VECTOR, intersect) {
	vecl::set_vector<int> a = { 1,2,3,4,5 };
	vecl::set_vector<int> b = { 6,7,8,2,5 };
	vecl::set_vector<int> c = { 2,5 };

	a.intersect(b);
	ASSERT_EQ(a.set_equal(c), true);
}

*/
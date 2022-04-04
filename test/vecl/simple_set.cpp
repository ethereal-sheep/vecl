
#include <vecl/simple_set.hpp>
#include <unordered_set>
#include <gtest/gtest.h>
#include <array>

using namespace vecl;



struct test {
	std::string id;
	std::string s;
	test(std::string i = "LOL") : id{ i }, s{ "Default" } {}
	test(const test& rhs) : id{ rhs.id }, s{ "Copy" } {}
	test& operator=(const test& rhs) { id = rhs.id; s = "Copy"; return *this; }
	test(test&& rhs) noexcept : id{ std::move(rhs.id) }, s{ "Move" } {}
	test& operator=(test&& rhs) noexcept { id = std::move(rhs.id); s = "Move"; return *this; }
	~test() {
		// std::cout << "Deleted " << s << " " << id << std::endl; 
	}

	bool operator==(const test& that) const {
		return id == that.id;
	}

};

namespace std
{
	template<>
	struct hash<test>
	{
		size_t operator()(const test& test) const noexcept
		{
			return std::hash<std::string>{}(test.id);
		}
	};
}


TEST(SIMPLE_SET, constructor) {
	vecl::simple_set<std::string> a;
	vecl::simple_set<int> b;
	vecl::simple_set<size_t> c;

	ASSERT_EQ(a.size(), 0);
}


struct identity
{
	size_t operator()(int a) const { return a; }
};

TEST(SIMPLE_SET, insert_int) {


	vecl::simple_set<int> a;
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
TEST(SIMPLE_SET, insert_string) {


	vecl::simple_set<std::string> a;
	{
		auto b = a.insert("0");
		ASSERT_EQ(b, true);
		ASSERT_EQ(a.size(), 1);
		ASSERT_EQ(a.count("0"), 1);
	}
	{
		auto b = a.insert("0");
		ASSERT_EQ(b, false);
		ASSERT_EQ(a.size(), 1);
		ASSERT_EQ(a.count("0"), 1);
	}
	{
		auto b = a.insert("1");
		ASSERT_EQ(b, true);
		ASSERT_EQ(a.size(), 2);
		ASSERT_EQ(a.count("1"), 1);
		std::cout << a << std::endl;
	}
}

TEST(SIMPLE_SET, erase) {


	vecl::simple_set<int> a;
	{
		a.insert(0);
		a.insert(1);
		a.insert(2);
		a.insert(3);
		a.insert(4);
		std::cout << a << std::endl;
	}
	{
		a.erase(0);
		ASSERT_EQ(a.count(0), 0);
		std::cout << a << std::endl;
	}
	{
		a.erase(1);
		ASSERT_EQ(a.count(1), 0);
		std::cout << a << std::endl;
	}
}


TEST(SIMPLE_SET, erase_string) {


	vecl::simple_set<std::string> a;
	{
		a.insert("1");
		a.insert("3");
		a.insert("5");
		a.insert("a");
		a.insert("s");
		a.insert("0");
		a.insert("f");
		a.insert("g");
		std::cout << a << std::endl;
	}
	{
		a.erase("0");
		ASSERT_EQ(a.count("0"), 0);
		std::cout << a << std::endl;
	}
	{
		a.erase("1");
		ASSERT_EQ(a.count("1"), 0);
		std::cout << a << std::endl;
	}
}

//TEST(SIMPLE_SET, simple_buffer_test) {
//	vecl::simple_buffer<std::string> a(10, "123");
//	ASSERT_EQ(a.size(), 10);
//
//	vecl::simple_buffer<int*> b(20, nullptr);
//
//	for (auto& i : b)
//	{
//		i = new int(1);
//	}
//
//	for (int i = 0; i < (int)b.size(); ++i)
//	{
//		ASSERT_EQ(*b[i], 1);
//	}
//	for (auto& i : b)
//	{
//		delete i;
//	}
//
//}

//TEST(SIMPLE_SET, stress_simple) {
//	vecl::simple_set<int> a;
//
//	for (auto i = 0; i < 10000; ++i)
//	{
//		a.insert(i);
//	}
//	for (auto i = 0; i < 5000; ++i)
//	{
//		a.erase(i);
//	}
//	for (auto i = 5001; i < 10000; ++i)
//	{
//		ASSERT_TRUE(a.contains(i));
//	}
//}
//TEST(SIMPLE_SET, stress_simple_unordered_set) {
//	std::unordered_set<int> a;
//
//	for (auto i = 0; i < 10000; ++i)
//	{
//		a.insert(i);
//	}
//	for (auto i = 0; i < 5000; ++i)
//	{
//		a.erase(i);
//	}
//	for (auto i = 5001; i < 10000; ++i)
//	{
//		ASSERT_TRUE(a.contains(i));
//	}
//}
//
//
//TEST(SIMPLE_SET, stress_string) {
//	vecl::simple_set<test> a;
//
//	for (auto i = 0; i < 10000; ++i)
//	{
//		a.insert(test(std::to_string(i)));
//	}
//	for (auto i = 0; i < 5000; ++i)
//	{
//		a.erase(test(std::to_string(i)));
//	}
//	for (auto i = 5001; i < 10000; ++i)
//	{
//		ASSERT_TRUE(a.contains(test(std::to_string(i))));
//	}
//}
//TEST(SIMPLE_SET, stress_string_unordered_set) {
//	std::unordered_set<test> a;
//
//	for (auto i = 0; i < 10000; ++i)
//	{
//		a.insert(test(std::to_string(i)));
//	}
//	for (auto i = 0; i < 5000; ++i)
//	{
//		a.erase(test(std::to_string(i)));
//	}
//	for (auto i = 5001; i < 10000; ++i)
//	{
//		ASSERT_TRUE(a.contains(test(std::to_string(i))));
//	}
//}
//
//TEST(SIMPLE_SET, range_constructor) {
//	std::vector<float> a;
//	int t = 50;
//	while (t--)
//		a.push_back(static_cast<float>(t));
//
//	vecl::simple_set<size_t> b(a.begin(), a.end());
//
//	ASSERT_EQ(b.size(), a.size());
//	for (auto i : a)
//		ASSERT_EQ(b.count(static_cast<uint32_t>(i)), 1);
//
//}
//
//TEST(SIMPLE_SET, capacity_range_constructor) {
//	std::vector<float> a;
//	int t = 50;
//	while (t--)
//		a.push_back(static_cast<float>(t));
//
//	vecl::simple_set<size_t> b(a.begin(), a.end(), 100);
//
//	ASSERT_EQ(b.capacity(), 100);
//	ASSERT_EQ(b.size(), a.size());
//	for (auto i : a)
//		ASSERT_EQ(b.count(static_cast<uint32_t>(i)), 1);
//
//}

TEST(SIMPLE_SET, il_constructor) {
	vecl::simple_set<size_t> b{ 1,2,3,4 };

	ASSERT_TRUE(b.count(1));
	ASSERT_TRUE(b.count(2));
	ASSERT_TRUE(b.count(3));
	ASSERT_TRUE(b.count(4));
}

TEST(SIMPLE_SET, capacity_il_constructor) {
	vecl::simple_set<size_t> b({ 1,2,3,4 }, 10);

	ASSERT_EQ(b.capacity(), 10);
	ASSERT_TRUE(b.count(1));
	ASSERT_TRUE(b.count(2));
	ASSERT_TRUE(b.count(3));
	ASSERT_TRUE(b.count(4));
}
//
//TEST(SIMPLE_SET, emplace_push_pop) {
//	vecl::simple_set a;
//
//	ASSERT_EQ(a.empty(), true);
//
//	auto [it, b] = a.emplace_back(0);
//	ASSERT_EQ(b, true);
//	ASSERT_EQ(*it, 0);
//	ASSERT_EQ(a.size(), 1);
//	ASSERT_EQ(a.count(0), 1);
//
//	a.pop_back();
//	ASSERT_EQ(a.size(), 0);
//	ASSERT_EQ(a.count(0), 0);
//
//	a.push_back(0);
//	ASSERT_EQ(a.size(), 1);
//	ASSERT_EQ(a.count(0), 1);
//
//}
//
//
//TEST(SIMPLE_SET, find) {
//	vecl::simple_set a;
//	{
//		auto it = a.find(2);
//		ASSERT_EQ(it, a.end());
//	}
//	a.emplace_back(0);
//	a.emplace_back(1);
//	a.emplace_back(2);
//	{
//		auto it = a.find(2);
//		ASSERT_NE(it, a.end());
//		ASSERT_EQ(*it, 2);
//	}
//}
//

//
//TEST(SIMPLE_SET, swap) {
//	vecl::simple_set a;
//	auto [ait, x] = a.emplace_back(0);
//	vecl::simple_set b;
//	auto [bit, y] = b.emplace_back(1);
//
//	std::swap(a, b);
//	ASSERT_EQ(*ait, 0);
//	ASSERT_EQ(*bit, 1);
//	ASSERT_EQ(a.count(1), 1);
//	ASSERT_EQ(b.count(0), 1);
//
//	swap(a, b);
//	ASSERT_EQ(*ait, 0);
//	ASSERT_EQ(*bit, 1);
//	ASSERT_EQ(b.count(1), 1);
//	ASSERT_EQ(a.count(0), 1);
//
//	a.swap(b);
//	ASSERT_EQ(*ait, 0);
//	ASSERT_EQ(*bit, 1);
//	ASSERT_EQ(a.count(1), 1);
//	ASSERT_EQ(b.count(0), 1);
//
//	b.swap(a);
//	ASSERT_EQ(*ait, 0);
//	ASSERT_EQ(*bit, 1);
//	ASSERT_EQ(b.count(1), 1);
//	ASSERT_EQ(a.count(0), 1);
//}
//
//TEST(SIMPLE_SET, clear) {
//	vecl::simple_set a;
//	int t = 1000;
//	while (--t)
//		a.push_back(t);
//	a.clear();
//
//	ASSERT_EQ(a.count(0), 0);
//	ASSERT_EQ(a.count(1), 0);
//	ASSERT_EQ(a.count(999), 0);
//	ASSERT_EQ(a.count(1000), 0);
//}
//
//TEST(SIMPLE_SET, equal) {
//	vecl::simple_set a;
//	vecl::simple_set b;
//	vecl::simple_set<size_t> c;
//	int t = 500;
//	while (t--)
//	{
//		a.push_back(t);
//		b.push_back(t);
//		c.push_back(t);
//	}
//	ASSERT_EQ(a, b);
//	ASSERT_EQ(a.set_equal(b), true);
//	ASSERT_EQ(vecl::set_equal(c, b), true);
//	ASSERT_EQ(vecl::set_equal(a, c, b), true);
//}
//
//TEST(SIMPLE_SET, sort) {
//	vecl::simple_set a = { 1,2,3,4,5 };
//	vecl::simple_set b = { 5,4,1,2,3 };
//
//	b.sort();
//
//	ASSERT_EQ(a, b);
//}
//
//TEST(SIMPLE_SET, merge) {
//	vecl::simple_set a = { 1,2,3,4,5 };
//	vecl::simple_set b = { 6,7,8,2,5 };
//	vecl::simple_set c = { 1,2,3,4,5,6,7,8 };
//
//	a.merge(b);
//	EXPECT_EQ(a, c);
//	ASSERT_EQ(a.set_equal(c), true);
//}
//
//TEST(SIMPLE_SET, intersect) {
//	vecl::simple_set a = { 1,2,3,4,5 };
//	vecl::simple_set b = { 6,7,8,2,5 };
//	vecl::simple_set c = { 2,5 };
//
//	a.intersect(b);
//	ASSERT_EQ(a.set_equal(c), true);
//}
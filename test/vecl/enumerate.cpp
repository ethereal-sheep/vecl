
#include <vecl/enumerate.hpp>
#include <gtest/gtest.h>

void print(auto e)
{
	for (auto i : e)
		std::cout << i << " ";
	std::cout << std::endl;
}


TEST(ENUMERATE, single_end) {

	int c = 0;
	for (auto i : vecl::enumerate(5))
		ASSERT_EQ(i, c++);

	print(vecl::enumerate(5));
}

TEST(ENUMERATE, start_end) {

	int start = 5;
	int n = 10;
	int c = start;
	auto e = vecl::enumerate(start, start + n);
	for (auto i : e)
		ASSERT_EQ(i, c++);

	print(e);
}

TEST(ENUMERATE, reverse_start_end) {

	int start = 5;
	int n = 10;
	int c = start + n;
	auto e = vecl::enumerate(start + n, start);
	for (auto i : e)
		ASSERT_EQ(i, c--);

	print(e);
}

TEST(ENUMERATE, iterator_compare) {

	auto compare = [](int start, int end)
	{
		auto en = vecl::enumerate(start, end);
		ASSERT_LE(en.begin(), en.end());
	};

	compare(0, 1);
	compare(1, 0);
	compare(-1, 1);
	compare(1, -1);
}

TEST(ENUMERATE, single_end_simple_xform) {

	auto compare = [](auto lambda)
	{
		int end = 10;
		int c = 0;
		auto en = vecl::enumerate(end, lambda);
		for (auto i : en)
			ASSERT_EQ(i, lambda(c++));

	};

	auto times_two = [](int i)
	{
		return i * 2;
	};

	compare(times_two);
}

TEST(ENUMERATE, start_end_simple_xform) {

	auto compare = [](auto lambda)
	{
		int start = -10;
		int end = 10;
		int c = start;
		auto en = vecl::enumerate(start, end, lambda);
		for (auto i : en)
			ASSERT_EQ(i, lambda(c++));
	};

	auto times_two = [](int i)
	{
		return i * 2;
	};

	auto times_three = [](int i)
	{
		return i * 3;
	};

	auto poly = [](int i)
	{
		return i * i * 3 + 5 + i;
	};

	auto special = [](auto i)
	{
		return i / 2.f;
	};

	compare(times_two);
	compare(times_three);
	compare(poly);
	compare(special);
}

TEST(ENUMERATE, start_end_special_xform) {

	auto compare = [](auto lambda)
	{
		int start = -10;
		int end = 10;
		int c = start;
		auto en = vecl::enumerate(start, end, lambda);
		for (auto [i, j] : en)
		{
			auto [x, y] = lambda(c++);
			ASSERT_EQ(i, x);
			ASSERT_EQ(j, y);
		}
	};

	auto split = [](int i)
	{
		return std::make_pair(i, i);
	};

	auto split_times_two = [](auto i)
	{
		return std::make_pair(i * 2.f, i / 2.f);
	};

	compare(split);
	compare(split_times_two);
}

TEST(ENUMERATE, start_end_test_intellisense) {


	auto split = [](int i)
	{
		return std::make_pair(i, i);
	};

	auto split_times_two = [](auto i)
	{
		return std::make_pair(i * 2.f, i / 2.f);
	};

	auto a = std::function<std::pair<int, int>(int)>(split);

	auto en = vecl::enumerate(10, 20, split);
}

TEST(ENUMERATE, start_end_dead_function) {

	int c = 5;
	auto en = vecl::enumerate(10);
	{
		int a = c;
		auto dead = [a](int i)
		{
			return i + a;
		};

		en = vecl::enumerate(10, dead);
	}

	int a = c;
	auto alive = [a](int i)
	{
		return i + a;
	};

	c = 0;

	for (auto i : en)
	{
		auto x = alive(c++);
		ASSERT_EQ(i, x);
	}
}
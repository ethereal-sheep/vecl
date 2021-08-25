#include <gtest/gtest.h>
#include <vecl/view.hpp>
#include <vector>

using vi = std::vector<int>;

TEST(VIEWS, view) {

	vi a = { 1,2,3,4,5 };
	auto b = vecl::view(a);
	auto c = vecl::view(a.begin(), a.end());

	auto x = 0;
	for (auto i : b)
	{
		ASSERT_EQ(a[x], i);
		++x;
	}
	x = 0;
	for (auto i : c)
	{
		ASSERT_EQ(a[x], i);
		++x;
	}

}

TEST(VIEWS, const_view) {

	const vi a{ 1,2,3,4,5 };
	auto b = vecl::view(a);
	auto c = vecl::view(a.begin(), a.end());

	auto x = 0;
	for (auto i : b)
	{
		ASSERT_EQ(a[x], i);
		++x;
	}

	x = 0;
	for (auto i : c)
	{
		ASSERT_EQ(a[x], i);
		++x;
	}

}

TEST(VIEWS, reverse_view) {

	const vi a{ 1,2,3,4,5 };
	auto b = vecl::reverse(a);
	auto c = vecl::view(a.rbegin(), a.rend());

	auto x = a.size() - 1;
	for (auto i : b)
	{
		ASSERT_EQ(a[x], i);
		--x;
	}

	x = a.size() - 1;
	for (auto i : c)
	{
		ASSERT_EQ(a[x], i);
		--x;
	}

}
TEST(VIEWS, skip_view) {

	const vi a{ 1,2,3,4,5 };
	auto b = vecl::skip(a, [](auto a) {return a == 2; });
	auto c = vecl::skip(a.begin(), a.end(), [](auto a) {return a == 2; });

	auto x = 0;
	for (auto i : b)
	{
		if (a[x] == 2)
			++x;

		ASSERT_EQ(a[x], i);
		++x;
	}

	x = 0;
	for (auto i : c)
	{
		if (a[x] == 2)
			++x;
		ASSERT_EQ(a[x], i);
		++x;
	}

}
TEST(VIEWS, drop_view) {

	vi a{ 1,2,3,4,5 };
	auto b = vecl::drop(a, 3.f);
	auto c = vecl::drop(a.begin(), a.end(), 2);

	auto x = 0;
	for (auto i : b)
	{
		if (a[x] == 3)
			++x;

		ASSERT_EQ(a[x], i);
		++x;
	}

	x = 0;
	for (auto i : c)
	{
		if (a[x] == 2)
			++x;
		ASSERT_EQ(a[x], i);
		++x;
	}

}
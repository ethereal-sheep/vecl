
#include <vector>
#include <unordered_set>
#include <vecl/pipes.hpp>
#include <gtest/gtest.h>
using namespace vecl::pipes;


TEST(PIPES, basic_transform_pipe) {

	std::vector<int> v = { 1,2,3,4,5 };

	auto lambda = [](int a) {
		return a * 2;
	};

	auto pipe = transform(lambda);

	auto end = pipe << v;

	for (int i = 0; i < end.size(); ++i)
	{
		ASSERT_EQ(end[0], lambda(v[0]));
	}
}
TEST(PIPES, combined_transform_pipe) {

	std::vector<int> v = { 1,2,3,4,5 };

	auto lambda1 = [](int a) {
		return a * 2;
	};
	auto lambda2 = [](int a) {
		return a + 2;
	};

	auto pipe1 = transform(lambda1);
	auto pipe2 = transform(lambda2);

	auto end = pipe2 + pipe1 << v;

	for (int i = 0; i < end.size(); ++i)
	{
		ASSERT_EQ(end[0], lambda2(lambda1(v[0])));
	}
}
TEST(PIPES, basic_transform_pipe_diff_type) {

	std::vector<int> v = { 1,2,3,4,5 };

	auto lambda1 = [](int a) {
		return a * 2;
	};
	auto lambda2 = [](int a) {
		return a + 2.f;
	};

	auto pipe2 = transform(lambda2);

	auto end = pipe2 << v;

	for (int i = 0; i < end.size(); ++i)
	{
		ASSERT_EQ(end[0], lambda2(v[0]));
	}
}
TEST(PIPES, combine_transform_pipe_diff_type) {

	std::vector<int> v = { 1,2,3,4,5 };

	auto lambda1 = [](int a) {
		return a * 2;
	};
	auto lambda2 = [](int a) {
		return float(a + 2.f);
	};
	auto lambda3 = [](float a) {
		return a * 2.5f;
	};

	auto pipe1 = transform(lambda1);
	auto pipe2 = transform(lambda2);
	auto pipe3 = transform(lambda3);

	auto combined = pipe3 + pipe2 + pipe1;

	auto end = combined << v;

	for (int i = 0; i < end.size(); ++i)
	{
		ASSERT_EQ(end[0], lambda3(lambda2(lambda1(v[0]))));
	}
}
TEST(PIPES, unordered_set_pipe) {

	std::unordered_set<int> v = { 1,2,3,4,5 };

	auto lambda1 = [](int a) {
		return a * 2;
	};
	auto lambda2 = [](int a) {
		return float(a + 2.f);
	};
	auto lambda3 = [](float a) {
		return a * 2.5f;
	};

	auto pipe1 = transform(lambda1);
	auto pipe2 = transform(lambda2);
	auto pipe3 = transform(lambda3);

	auto combined = pipe3 + pipe2 + pipe1;

	auto end = combined << v;

	for (auto i : v)
	{
		ASSERT_TRUE(end.contains(lambda3(lambda2(lambda1(i)))));
	}
}

TEST(PIPES, combined_transform_filter_pipe) {

	std::vector<int> v = { 1,2,3,4,5 };

	auto lambda1 = [](int a) {
		return a * 2;
	};
	auto lambda2 = [](int a) {
		return a % 2 == 0;
	};

	auto pipe1 = transform(lambda1);
	auto pipe2 = filter(lambda2);

	auto end = pipe2 + pipe1 << v;

	ASSERT_EQ(end.size(), 0);
}
TEST(PIPES, combined_filter_transform_pipe) {

	std::vector<int> v = { 1,2,3,4,5 };

	auto lambda1 = [](int a) {
		return a * 2;
	};
	auto lambda2 = [](int a) {
		return a % 2 == 0;
	};

	auto pipe1 = transform(lambda1);
	auto pipe2 = filter(lambda2);

	auto end = pipe1 + pipe2 << v;

	ASSERT_EQ(end.size(), 3);
}

TEST(PIPES, replace_pipe) {

	std::vector<int> v = { 1,2,3,4,5 };

	auto end = replace(3.0) << v;

	ASSERT_EQ(end.size(), 5);
	for (auto i : end)
	{
		ASSERT_EQ(i, 3);
	}
}
TEST(PIPES, replace_if_pipe) {

	std::vector<int> v = { 1,2,3,4,5 };

	auto end = replace_if([](int) { return true; }, 4.01) << v;

	ASSERT_EQ(end.size(), 5);
	for (auto i : end)
	{
		ASSERT_EQ(i, 4.01);
	}
}
TEST(PIPES, inner_product_pipe) {

	std::vector<int> v = { 1,2,3,4,5 };

	auto end = replace_if([](int) { return true; }, 4.01) << v;

	ASSERT_EQ(end.size(), 5);
	for (auto i : end)
	{
		ASSERT_EQ(i, 4.01);
	}
}
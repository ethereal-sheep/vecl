#include <gtest/gtest.h>
#include <vector>
#include <vecl/pipe/pipe_base.hpp>

using namespace vecl;


TEST(PIPES, pipe) {

	std::vector<int> v = { 1,2,3,4,5 };

	auto lambda = [](int a) {
		return a * 2;
	};

	auto pipe = vecl::pipe::transform(lambda);

	auto end = pipe << v;
	
	/*for (int i = 0; i < end.size(); ++i)
	{
		ASSERT_EQ(end[0], lambda(lambda(v[0])));
	}*/
}
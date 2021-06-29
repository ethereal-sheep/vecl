#include <gtest/gtest.h>
#include <vecl/sparse_map.hpp>


TEST(SPARSE_MAP, constructor) {
	vecl::sparse_map<uint32_t, std::string> a = { {1, "HI"} };
}

#include <vecl/lru_cache.hpp>
#include <gtest/gtest.h>

using namespace vecl;

TEST(LRU_CACHE, constructor) {

	lru_cache<std::string, int> a(5);

}

TEST(LRU_CACHE, set) {

	lru_cache<std::string, int> a(5);
	a.set("hello", 1);
	ASSERT_TRUE(a.contains("hello"));

	a.set("hello_world", 2);
	ASSERT_TRUE(a.contains("hello_world"));

}
TEST(LRU_CACHE, get) {

	lru_cache<std::string, int> a(5);
	a.set("hello", 1);
	ASSERT_EQ(a.get("hello"), 1);

	a.set("hello_world", 2);
	ASSERT_EQ(a.get("hello_world"), 2);

	a.set("hello_world", 3);
	ASSERT_EQ(a.get("hello_world"), 3);
}

TEST(LRU_CACHE, evict) {

	lru_cache<std::string, int> a(5);
	a.set("hello", 1);
	ASSERT_EQ(a.get("hello"), 1);

	a.set("hello_world", 2);
	ASSERT_EQ(a.get("hello_world"), 2);

	a.set("hello_world", 3);
	ASSERT_EQ(a.get("hello_world"), 3);
}
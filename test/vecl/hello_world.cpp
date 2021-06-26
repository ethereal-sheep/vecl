#include <gtest/gtest.h>

// Demonstrate some basic assertions.
TEST(HelloWorld, BasicAssertions) {
	// Expect two strings not to be equal.
	EXPECT_STRNE("hello", "world");
}
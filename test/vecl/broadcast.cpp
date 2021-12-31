
#include <vecl/broadcast.hpp>
#include <vecl/memory.hpp>

#include <gtest/gtest.h>

using namespace std;


struct obj
{
	void foo() {
		++a;
	}

	~obj() { a = -1; }

	int a = 0;
};

int global_a = 0;
void goo()
{
	++global_a;
}

using bc = vecl::broadcast<std::function<void()>>;

TEST(BROADCAST, broadcast_constructor) {

	vecl::memory_malloc a;
	bc b(&a);
}

TEST(BROADCAST, broadcast_listen) {

	bc b;

	auto token = b.listen(goo);
	auto token2 = b.listen(goo);
	auto token3 = b.listen(goo);

	ASSERT_EQ(b.size(), 3);

}

TEST(BROADCAST, broadcast_copy) {

	vecl::memory_malloc a;
	bc b(&a), c(&a);

	auto i = c.listen(goo);
	bc d(c);
	b = c;

	ASSERT_EQ(b.size(), 1);
	ASSERT_EQ(d.size(), 1);

}

TEST(BROADCAST, broadcast_move) {

	vecl::memory_malloc a;
	bc b(&a), c(&a);

	auto i = c.listen(goo);

	b = std::move(c);

	ASSERT_EQ(b.size(), 1);
	ASSERT_EQ(c.size(), 0);

	std::swap(b, c);

	ASSERT_EQ(b.size(), 0);
	ASSERT_EQ(c.size(), 1);

	bc d(std::move(c));
	ASSERT_EQ(d.size(), 1);
	ASSERT_EQ(c.size(), 0);

	/*bc e;
	e = std::move(d);
	ASSERT_EQ(d.size(), 0);
	ASSERT_EQ(e.size(), 1);*/

}


TEST(BROADCAST, broadcast_trigger) {

	bc b;

	int ans = 0;
	auto test = [&ans]() {
		++ans;
	};


	auto token = b.listen(test);
	auto token2 = b.listen(test);
	auto token3 = b.listen(test);

	auto token4 = b.listen(goo);
	auto token5 = b.listen(goo);
	auto token6 = b.listen(goo);

	obj o;
	auto binded = std::bind(&obj::foo, &o);
	auto token7 = b.listen(binded);
	auto token8 = b.listen(binded);
	auto token9 = b.listen(binded);

	global_a = 0;
	b.trigger();

	ASSERT_EQ(ans, 3);
	ASSERT_EQ(o.a, 3);
	ASSERT_EQ(global_a, 3);
}

TEST(BROADCAST, broadcast_cleanup) {

	bc b;

	int ans = 0;
	auto test = [&ans]() {
		++ans;
	};


	{ auto token = b.listen(test); }
	{ auto token2 = b.listen(test); }
	auto token3 = b.listen(test);

	auto token4 = b.listen(goo);
	{ auto token5 = b.listen(goo);
	auto token6 = b.listen(goo); }

	obj o;
	auto binded = std::bind(&obj::foo, &o);
	{ auto token7 = b.listen(binded);
	auto token8 = b.listen(binded); }
	auto token9 = b.listen(binded);

	global_a = 0;
	b.trigger();

	ASSERT_EQ(ans, 1);
	ASSERT_EQ(o.a, 1);
	ASSERT_EQ(global_a, 1);
}


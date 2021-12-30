
#include <vecl/publisher.hpp>
#include <vecl/memory.hpp>
#include <gtest/gtest.h>

using namespace std;

struct test : public vecl::simple_message
{
	int a = 0;
};

struct obj
{
	void foo(const vecl::simple_message&) {
		++a;
	}

	~obj() { a = -1; }
	
	int a = 0;
};

int global_a = 0;

void goo(const vecl::simple_message&)
{
	++global_a;
}


TEST(PUBLISHER, publisher_constructor) {

	vecl::memory_malloc a;
	vecl::publisher def;
	vecl::publisher p(&a);
}

TEST(PUBLISHER, publisher_subscribe) {

	vecl::publisher p;

	auto i1 = p.subscribe<test>(goo);
	auto i2 = p.subscribe<test>(goo);
	auto i3 = p.subscribe<test>(goo);

	ASSERT_EQ(p.size<test>(), 3);
}

TEST(PUBLISHER, publisher_copy) {

	vecl::memory_malloc a;
	vecl::publisher p, g;

	auto i1 = p.subscribe<test>(goo);

	ASSERT_EQ(p.size<test>(), 1);
	ASSERT_EQ(g.size<test>(), 0);

	g = p;
	ASSERT_EQ(p.size<test>(), 1);
	ASSERT_EQ(g.size<test>(), 1);

}

TEST(PUBLISHER, publisher_move) {

	vecl::memory_malloc a;
	[[maybe_unused]]  vecl::publisher p(&a), g(&a), h;

	auto i1 = p.subscribe<test>(goo);

	ASSERT_EQ(p.size<test>(), 1);
	ASSERT_EQ(g.size<test>(), 0);

	g = std::move(p);
	ASSERT_EQ(p.size<test>(), 0);
	ASSERT_EQ(g.size<test>(), 1);

	/*h = std::move(g);
	ASSERT_EQ(g.size<test>(), 0);
	ASSERT_EQ(h.size<test>(), 1);*/
}


TEST(PUBLISHER, publisher_publish) {

	vecl::publisher p;

	int ans = 0;
	auto test_fn = [&ans](const vecl::simple_message&) {
		++ans;
	};


	auto token = p.subscribe<test>(test_fn);
	auto token2 = p.subscribe<test>(test_fn);
	auto token3 = p.subscribe<vecl::simple_message>(test_fn);

	auto token4 = p.subscribe<test>(goo);
	auto token5 = p.subscribe<test>(goo);
	auto token6 = p.subscribe<vecl::simple_message>(goo);

	obj o;
	auto binded = std::bind(&obj::foo, &o, std::placeholders::_1);
	auto token7 = p.subscribe<test>(binded);
	auto token8 = p.subscribe<test>(&obj::foo, &o);
	auto token9 = p.subscribe<vecl::simple_message>(binded);

	global_a = 0;
	p.publish<test>();

	ASSERT_EQ(ans, 2);
	ASSERT_EQ(o.a, 2);
	ASSERT_EQ(global_a, 2);


	p.publish<vecl::simple_message>();
	ASSERT_EQ(ans, 3);
	ASSERT_EQ(o.a, 3);
	ASSERT_EQ(global_a, 3);

}


TEST(PUBLISHER, publisher_cleanup) {

	vecl::publisher p;

	int ans = 0;
	auto test_fn = [&ans](const vecl::simple_message&) {
		++ans;
	};

	{
		{auto token = p.subscribe<test>(test_fn);
		auto token2 = p.subscribe<test>(test_fn); }
		auto token3 = p.subscribe<vecl::simple_message>(test_fn);

		auto token4 = p.subscribe<test>(goo);
		{auto token5 = p.subscribe<test>(goo);
		auto token6 = p.subscribe<vecl::simple_message>(goo); }

		obj o;
		auto binded = std::bind(&obj::foo, &o, std::placeholders::_1);
		{auto token7 = p.subscribe<test>(binded); }
		auto token8 = p.subscribe<test>(&obj::foo, &o);
		{auto token9 = p.subscribe<vecl::simple_message>(binded); }

		global_a = 0;
		p.publish<test>();

		ASSERT_EQ(ans, 0);
		ASSERT_EQ(o.a, 1);
		ASSERT_EQ(global_a, 1);


		p.publish<vecl::simple_message>();
		ASSERT_EQ(ans, 1);
		ASSERT_EQ(o.a, 1);
		ASSERT_EQ(global_a, 1);
	}

	ans = 0;
	global_a = 0;

	p.publish<test>();
	p.publish<vecl::simple_message>();

	ASSERT_EQ(ans, 0);
	ASSERT_EQ(global_a, 0);
}


TEST(PUBLISHER, publisher_schedule) {
	vecl::publisher p;

	int ans = 0;
	auto test_fn = [&ans](const vecl::simple_message&) {
		++ans;
	};


	auto token = p.subscribe<test>(test_fn);
	auto token2 = p.subscribe<test>(test_fn);
	auto token3 = p.subscribe<vecl::simple_message>(test_fn);

	auto token4 = p.subscribe<test>(goo);
	auto token5 = p.subscribe<test>(goo);
	auto token6 = p.subscribe<vecl::simple_message>(goo);

	obj o;
	auto binded = std::bind(&obj::foo, &o, std::placeholders::_1);
	auto token7 = p.subscribe<test>(binded);
	auto token8 = p.subscribe<test>(&obj::foo, &o);
	auto token9 = p.subscribe<vecl::simple_message>(binded);

	global_a = 0;
	p.schedule<test>();

	ASSERT_EQ(ans, 0);
	ASSERT_EQ(o.a, 0);
	ASSERT_EQ(global_a, 0);


	p.schedule<vecl::simple_message>();
	ASSERT_EQ(ans, 0);
	ASSERT_EQ(o.a, 0);
	ASSERT_EQ(global_a, 0);

	p.blast();

	ASSERT_EQ(ans, 3);
	ASSERT_EQ(o.a, 3);
	ASSERT_EQ(global_a, 3);


}
TEST(PUBLISHER, token_call) {

	int ans = 0;
	auto test_fn = [&ans](const vecl::simple_message&) {
		++ans;
	};

	vecl::publisher p;

	auto token = p.subscribe<test>(test_fn);

	ASSERT_EQ(ans, 0);
	(*token)(test());

	ASSERT_EQ(ans, 1);
}
TEST(PUBLISHER, token_reset) {

	int ans = 0;
	auto test_fn = [&ans](const vecl::simple_message&) {
		++ans;
	};

	vecl::publisher p;

	auto token = p.subscribe<test>(test_fn);

	ASSERT_EQ(ans, 0);

	p.publish<test>();

	ASSERT_EQ(ans, 1);

	token.reset();
	p.publish<test>();

	ASSERT_EQ(ans, 1);
}

TEST(PUBLISHER, SFINAE) {

	int ans = 0;
	auto test_fn = [&ans](const vecl::simple_message&) {
		++ans;
	};

	vecl::publisher p;

	struct not_derived : vecl::simple_message {

	};

	[[maybe_unused]]auto token = p.subscribe<not_derived>(test_fn);

}
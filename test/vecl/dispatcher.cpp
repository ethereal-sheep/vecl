#include <gtest/gtest.h>
#include <vecl/publisher.hpp>
#include <vecl/broadcast.hpp>
#include <vecl/memory.hpp>


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

using bc = vecl::broadcast<std::function<void(const vecl::simple_message&)>>;

TEST(DISPATCHER, broadcast_constructor) {

	//vecl::memory_debug a("constructor", vecl::get_default_memory_debug_callback());
	vecl::memory_malloc a;
	vecl::broadcast<std::function<void()>> b(&a);
	vecl::broadcast<std::function<void(const vecl::simple_message&)>> c(&a);
}

TEST(DISPATCHER, broadcast_copy) {

	//vecl::memory_debug a("constructor", vecl::get_default_memory_debug_callback());
	vecl::memory_malloc a;
	bc b(&a), c(&a);

	auto i = c.listen(goo);
	bc d(c);
	b = c;

	ASSERT_EQ(b.size(), 1);
	ASSERT_EQ(d.size(), 1);

}
TEST(DISPATCHER, broadcast_move) {

	//vecl::memory_debug a("constructor", vecl::get_default_memory_debug_callback());
	vecl::memory_malloc a;
	vecl::broadcast<std::function<void(const vecl::simple_message&)>> b(&a), c(&a);

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

	bc e;
	e = std::move(d);
	ASSERT_EQ(d.size(), 1);
	ASSERT_EQ(e.size(), 1);

}

TEST(DISPATCHER, broadcast_listen) {

	//vecl::memory_debug a("constructor", vecl::get_default_memory_debug_callback());

	vecl::broadcast<std::function<void(const vecl::simple_message&)>> bc;// (&a);

	auto token = bc.listen(goo);
	auto token2 = bc.listen(goo);
	auto token3 = bc.listen(goo);


}

TEST(DISPATCHER, broadcast_trigger) {

	//vecl::memory_debug a("constructor", vecl::get_default_memory_debug_callback());

	vecl::broadcast<std::function<void(const vecl::simple_message&)>> bc;

	int ans = 0;
	auto test = [&ans](const vecl::simple_message&) {
		++ans;
	};


	auto token = bc.listen(test);
	auto token2 = bc.listen(test);
	auto token3 = bc.listen(test);

	auto token4 = bc.listen(goo);
	auto token5 = bc.listen(goo);
	auto token6 = bc.listen(goo);

	obj o;

	auto binded = std::bind(&obj::foo, &o, std::placeholders::_1);
	auto token7 = bc.listen(binded);
	auto token8 = bc.listen(binded);
	auto token9 = bc.listen(binded);

	global_a = 0;
	bc.trigger(vecl::simple_message());

	ASSERT_EQ(ans, 3);
	ASSERT_EQ(o.a, 3);
	ASSERT_EQ(global_a, 3);
}

TEST(DISPATCHER, broadcast_cleanup) {

	//vecl::memory_debug a("constructor", vecl::get_default_memory_debug_callback());

	vecl::broadcast<std::function<void(const vecl::simple_message&)>> bc;

	int ans = 0;
	auto test = [&ans](const vecl::simple_message&) {
		++ans;
	};


	{ auto token = bc.listen(test); }
	{ auto token2 = bc.listen(test); }
	auto token3 = bc.listen(test);

	auto token4 = bc.listen(goo);
	{ auto token5 = bc.listen(goo);
	auto token6 = bc.listen(goo); }

	obj o;
	auto binded = std::bind(&obj::foo, &o, std::placeholders::_1);
	{ auto token7 = bc.listen(binded);
	auto token8 = bc.listen(binded); }
	auto token9 = bc.listen(binded);

	global_a = 0;
	bc.trigger(vecl::simple_message());

	ASSERT_EQ(ans, 1);
	ASSERT_EQ(o.a, 1);
	ASSERT_EQ(global_a, 1);
}




TEST(DISPATCHER, publisher_constructor) {

	vecl::memory_malloc a;
	vecl::publisher def;
	vecl::publisher p(&a);

	def = std::move(p);
}



TEST(DISPATCHER, publisher_memory) {

	//vecl::memory_debug a("abc", vecl::get_default_memory_debug_callback());
	vecl::publisher p;// (&a);


	auto i1 = p.subscribe<test>(goo);
	auto i2 = p.subscribe<test>(goo);
	auto i3 = p.subscribe<test>(goo);
	auto i4 = p.subscribe<test>(goo);
	auto i5 = p.subscribe<test>(goo);
	auto i6 = p.subscribe<test>(goo);
	auto i7 = p.subscribe<test>(goo);
	auto i8 = p.subscribe<test>(goo);

}

TEST(DISPATCHER, publisher_subscribe) {

	//vecl::memory_debug a("constructor", vecl::get_default_memory_debug_callback());
	
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


TEST(DISPATCHER, publisher_cleanup) {

	//vecl::memory_debug a("constructor", vecl::get_default_memory_debug_callback());

	vecl::publisher p;

	int ans = 0;
	auto test_fn = [&ans](const vecl::simple_message&) {
		++ans;
	};


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
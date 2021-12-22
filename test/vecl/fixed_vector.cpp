#include <gtest/gtest.h>
#include <vecl/fixed_vector.hpp>

#include <array>

struct test {
	std::string id;
	std::string s;
	test(std::string i = "LOL") : id{i}, s{"Default"} {}
	test(const test& rhs) : id{ rhs.id }, s{ "Copy" } {}
	test& operator=(const test& rhs) { id = rhs.id; s = "Copy"; return *this; }
	test(test&& rhs) noexcept : id{ std::move(rhs.id) }, s{ "Move" } {}
	test& operator=(test&& rhs) noexcept { id = std::move(rhs.id); s = "Move"; return *this; }
	~test() { 
		//std::cout << "Deleted " << s << " " << id << std::endl; 
	}

	bool operator==(const test& that) const {
		return id == that.id;
	}
	
};

void print(const vecl::fixed_vector<int, 16>& a)
{
	for (auto i : a)
		std::cout << i << " ";
	std::cout << std::endl;
}

void print(const vecl::fixed_vector<test, 16>& a)
{
	for (auto& i : a)
		std::cout << i.id << " ";
	std::cout << std::endl;
}
void print(const std::vector<int>& a)
{
	for (auto i : a)
		std::cout << i << " ";
	std::cout << std::endl;
}

TEST(FIXED_VECTOR, default_constructor) {
	vecl::fixed_vector<int, 32> a;

	static_assert(a.max_size() == 32);
	ASSERT_EQ(a.size(), 0);
	ASSERT_EQ(a.max_size(), 32);
	ASSERT_TRUE(a.empty());
}

TEST(FIXED_VECTOR, il_constructor) {
	vecl::fixed_vector<int, 16> a{ 1,2,3 };

	ASSERT_EQ(a.size(), 3);
	ASSERT_FALSE(a.empty());
}

TEST(FIXED_VECTOR, object_destruction) {

	vecl::fixed_vector<test, 32> a{test("1"), test("2")};
	a.clear();
}

TEST(FIXED_VECTOR, element_access) {
	vecl::fixed_vector<int, 16> a{ 1,2,3 };
	const vecl::fixed_vector<int, 16> b{ 1,2,3 };

	ASSERT_EQ(a[0], 1);
	ASSERT_EQ(a.at(2), 3);
	ASSERT_EQ(b[0], 1);
	ASSERT_EQ(b.at(2), 3);

	a[1] = 3;
	ASSERT_EQ(a[1], 3);
	a.at(1) = 1;
	ASSERT_EQ(a.at(1), 1);

	ASSERT_THROW([[maybe_unused]] auto temp = a.at(3), std::out_of_range);
	ASSERT_THROW([[maybe_unused]] auto temp = b.at(3), std::out_of_range);

	ASSERT_EQ(a.front(), 1);
	ASSERT_EQ(a.back(), 3);
}

TEST(FIXED_VECTOR, il_assignment) {
	vecl::fixed_vector<int, 16> a = { 1,2,3 };

	ASSERT_EQ(a.size(), 3);
	ASSERT_FALSE(a.empty());
	ASSERT_EQ(a[0], 1);

	a.assign({ 5,4,3,2,1 });

	ASSERT_EQ(a.size(), 5);
	ASSERT_FALSE(a.empty());
	ASSERT_EQ(a[0], 5);

	vecl::fixed_vector<int, 1> b;
	ASSERT_THROW(b.assign({1,2,3}), std::length_error);

	vecl::fixed_vector<int, 1, false> c;
	ASSERT_NO_THROW(c.assign({ 1,2,3 }));
	ASSERT_EQ(c[0], 1);
}

TEST(FIXED_VECTOR, il_appending) {
	vecl::fixed_vector<int, 16> a{ 1,2,3 };

	ASSERT_EQ(a.size(), 3);
	ASSERT_FALSE(a.empty());
	ASSERT_EQ(a[0], 1);

	a.append({ 4,5 });

	ASSERT_EQ(a.size(), 5);
	ASSERT_FALSE(a.empty());
	ASSERT_EQ(a[4], 5);
}

TEST(FIXED_VECTOR, copy_constructor) {
	vecl::fixed_vector<int, 16> a{ 1,2,3 };
	vecl::fixed_vector<int, 16> b(a);

	ASSERT_EQ(b.size(), 3);
	ASSERT_FALSE(b.empty());
	for (int i = 0; i < a.size(); ++i)
		ASSERT_EQ(b[i], a[i]);
}

TEST(FIXED_VECTOR, copy_assignment) {
	vecl::fixed_vector<int, 16> a{ 1,2,3 };

	{
		vecl::fixed_vector<int, 16> b = a;
		ASSERT_EQ(b.size(), 3);
		ASSERT_FALSE(b.empty());
		for (int i = 0; i < a.size(); ++i)
			ASSERT_EQ(b[i], a[i]);
	}
	{
		vecl::fixed_vector<int, 16> b;
		b.assign(a);
		ASSERT_EQ(b.size(), 3);
		ASSERT_FALSE(b.empty());
		for (int i = 0; i < a.size(); ++i)
			ASSERT_EQ(b[i], a[i]);
	}
}

TEST(FIXED_VECTOR, move_constructor) {
	vecl::fixed_vector<int, 16> a{ 1,2,3 };
	vecl::fixed_vector<int, 16> b(a);
	vecl::fixed_vector<int, 16> c(std::move(b));

	ASSERT_EQ(b.size(), 0);
	ASSERT_TRUE(b.empty());

	ASSERT_EQ(c.size(), 3);
	ASSERT_FALSE(c.empty());
	for (int i = 0; i < a.size(); ++i)
		ASSERT_EQ(a[i], c[i]);

}

TEST(FIXED_VECTOR, move_assignment) {
	vecl::fixed_vector<int, 16> a{ 1,2,3 };

	{
		vecl::fixed_vector<int, 16> b(a);
		vecl::fixed_vector<int, 16> c = std::move(b);

		ASSERT_EQ(b.size(), 0);
		ASSERT_TRUE(b.empty());

		ASSERT_EQ(c.size(), 3);
		ASSERT_FALSE(c.empty());
		for (int i = 0; i < a.size(); ++i)
			ASSERT_EQ(a[i], c[i]);
	}
}

TEST(FIXED_VECTOR, explicit_constructor) {
	vecl::fixed_vector<int, 16> a(5, 1);
	ASSERT_EQ(a.size(), 5);
	ASSERT_FALSE(a.empty());
	for (int i = 0; i < a.size(); ++i)
		ASSERT_EQ(a[i], 1);
}

TEST(FIXED_VECTOR, explicit_assignment) {
	vecl::fixed_vector<int, 16> a;
	a.assign(5, 1);
	ASSERT_EQ(a.size(), 5);
	ASSERT_FALSE(a.empty());
	for (int i = 0; i < a.size(); ++i)
		ASSERT_EQ(a[i], 1);
}

TEST(FIXED_VECTOR, range_constructor) {
	std::vector<int> b(5,1);
	vecl::fixed_vector<int, 16> a(b.begin(), b.end());

	ASSERT_EQ(a.size(), b.size());
	for (int i = 0; i < a.size(); ++i)
		ASSERT_EQ(a[i], b[i]);
}

TEST(FIXED_VECTOR, range_assignment) {
	std::vector<int> b(5, 1);
	vecl::fixed_vector<int, 16> a;
	a.assign(b.begin(), b.end());

	ASSERT_EQ(a.size(), b.size());
	for (int i = 0; i < a.size(); ++i)
		ASSERT_EQ(a[i], b[i]);
}

TEST(FIXED_VECTOR, pop_back) {
	vecl::fixed_vector<int, 16> a{ 1,2,3 };
	ASSERT_EQ(a.size(), 3);
	ASSERT_EQ(a.back(), 3);
	a.pop_back();

	ASSERT_EQ(a.size(), 2);
	ASSERT_EQ(a.back(), 2);

}

TEST(FIXED_VECTOR, insert_one) {
	vecl::fixed_vector<int, 16> a{ 1,2,3 };
	ASSERT_EQ(a.size(), 3);
	ASSERT_EQ(a.back(), 3);

	auto it = a.insert(a.begin(), 5);

	ASSERT_EQ(a.size(), 4);
	ASSERT_EQ(a.front(), 5);
	ASSERT_EQ(*it, 5);

	it = a.insert(a.begin() + 3, 15);

	ASSERT_EQ(a.size(), 5);
	ASSERT_EQ(*it, 15);

	it = a.insert(a.end(), 21);

	ASSERT_EQ(a.size(), 6);
	ASSERT_EQ(a.back(), 21);
	ASSERT_EQ(*it, 21);

	//print(a);

}

TEST(FIXED_VECTOR, insert_one_internal_move) {
	vecl::fixed_vector<test, 16> a{ test("1"), test("1"), test("1") };
	ASSERT_EQ(a.size(), 3);

	auto it = a.insert(a.begin(), test("2"));

	ASSERT_EQ(a.size(), 4);
	ASSERT_EQ(a.front().id, "2");
	ASSERT_EQ(it->id, "2");

	it = a.insert(a.begin() + 2, test("3"));

	ASSERT_EQ(a.size(), 5);
	ASSERT_EQ(it->id, "3");

	auto old = it;
	it = a.insert(a.begin() + 3, std::move(*it));

	ASSERT_EQ(a.size(), 6);
	ASSERT_EQ(it->id, "3");
	ASSERT_EQ(old->id, "");

	//print(a);
}

TEST(FIXED_VECTOR, insert_many) {

	vecl::fixed_vector<int, 16> a{ 1,2,3 };
	ASSERT_EQ(a.size(), 3);
	ASSERT_EQ(a.back(), 3);

	auto it = a.insert(a.begin(), 2, 5);

	ASSERT_EQ(a.size(), 5);
	ASSERT_EQ(a.front(), 5);
	ASSERT_EQ(*it, 5);

	it = a.insert(a.begin() + 3, 4, 15);

	ASSERT_EQ(a.size(), 9);
	ASSERT_EQ(*it, 15);

	it = a.insert(a.begin() + 3, 0, 15);

	ASSERT_EQ(a.size(), 9);
	ASSERT_EQ(*it, 15);

	//print(a);

}

TEST(FIXED_VECTOR, insert_many_internal_ref) {

	vecl::fixed_vector<int, 16> a{ 1,2,3 };
	ASSERT_EQ(a.size(), 3);
	ASSERT_EQ(a.back(), 3);

	auto it = a.insert(a.begin(), 5, a.back());

	ASSERT_EQ(a.size(), 8);
	ASSERT_EQ(a.front(), 3);
	ASSERT_EQ(a.back(), 3);
	ASSERT_EQ(*it, 3);

	//print(a);
}

TEST(FIXED_VECTOR, insert_many_internal_move) {
	vecl::fixed_vector<test, 16> a{ test("1"), test("1"), test("3") };
	ASSERT_EQ(a.size(), 3);

	auto it = a.insert(a.begin(), 5, std::move(*(a.end() - 1)));
	//print(a);

	ASSERT_EQ(a.size(), 8);
	ASSERT_EQ(a.front().id, "3");
	ASSERT_EQ(a.back().id, "3");
	ASSERT_EQ(it->id, "3");

}


TEST(FIXED_VECTOR, insert_iterator_range) {

	vecl::fixed_vector<int, 16> a{ 0,1,2,3,4,5 };
	std::vector<int> b{ 0,1,2,3,4,5 };

	auto insert = [](auto& a) {
		a.insert(a.begin() + 4, a.begin() + 1, a.begin() + 6);
	};

	auto equal = [](auto& a, auto& b) {
		if (a.size() != b.size()) return false;

		for (auto i = 0; i < a.size(); ++i)
			if (a[i] != b[i])
				return false;
		return true;
	};

	insert(a);
	insert(b);

	/*print(a);
	print(b);*/

	ASSERT_TRUE(equal(a, b));

}

TEST(FIXED_VECTOR, insert_iterator_range_test) {

	auto insert = [](auto& a, int it, int from, int to) {
		a.insert(a.begin() + it, a.begin() + from, a.begin() + to);
	};

	auto equal = [](auto& a, auto& b) {
		if (a.size() != b.size()) return false;

		for (auto i = 0; i < a.size(); ++i)
			if (a[i] != b[i])
				return false;
		return true;
	};

	auto test_insert = [&](int it, int from, int to) {

		auto il = { test("0"), test("1"), test("2"), test("3"), test("4"), test("5") };
		vecl::fixed_vector<test, 16> a{ il.begin(), il.end() };
		std::vector<test> b{ il.begin(), il.end() };

		insert(a, it, from, to);
		insert(b, it, from, to);

		/*print(a);

		std::cout << "\nKilling fv:\n";
		a.clear();
		std::cout << "\nKilling v:\n";
		b.clear();
		std::cout << "\nKilling il:\n";*/

		ASSERT_TRUE(equal(a, b));
	};

	test_insert(0, 1, 2);
	test_insert(2, 1, 6);
	test_insert(4, 1, 6);
	test_insert(6, 1, 5);
	test_insert(4, 1, 1);

}

TEST(FIXED_VECTOR, insert_iterator_external_test) {

	auto insert = [](auto& a, int it, auto from, auto to) {
		a.insert(a.begin() + it, from, to);
	};

	auto equal = [](auto& a, auto& b) {
		if (a.size() != b.size()) return false;

		for (auto i = 0; i < a.size(); ++i)
			if (a[i] != b[i])
				return false;
		return true;
	};

	auto test_insert = [&](int it, int from, int to) {

		auto il = { test("0"), test("1"), test("2"), test("3"), test("4"), test("5") };
		vecl::fixed_vector<test, 16> a{ il.begin(), il.end() };
		std::vector<test> b{ il.begin(), il.end() };

		insert(a, it, il.begin() + from, il.begin() + to);
		insert(b, it, il.begin() + from, il.begin() + to);

		/*print(a);
		std::cout << "\nKilling fv:\n";
		a.clear();
		std::cout << "\nKilling v:\n";
		b.clear();
		std::cout << "\nKilling il:\n";*/

		ASSERT_TRUE(equal(a, b));
	};

	test_insert(0, 1, 2);
	test_insert(2, 1, 1);
	test_insert(4, 1, 6);
	test_insert(6, 1, 5);
	test_insert(4, 1, 3);

}

TEST(FIXED_VECTOR, insert_iterator_il) {

	auto insert = [](auto& a, int it, auto& il) {
		a.insert(a.begin() + it, il);
	};

	auto equal = [](auto& a, auto& b) {
		if (a.size() != b.size()) return false;

		for (auto i = 0; i < a.size(); ++i)
			if (a[i] != b[i])
				return false;
		return true;
	};

	auto test_insert = [&](int it, auto& il) {

		vecl::fixed_vector<test, 16> a{ il.begin(), il.end() };
		std::vector<test> b{ il.begin(), il.end() };

		insert(a, it, il);
		insert(b, it, il);

		/*print(a);
		std::cout << "\nKilling fv:\n";
		a.clear();
		std::cout << "\nKilling v:\n";
		b.clear();
		std::cout << "\nKilling il:\n";*/

		ASSERT_TRUE(equal(a, b));
	};

	auto il1 = { test("0"), test("1"), test("2"), test("3"), test("4"), test("5") };
	auto il2 = { test("1"), test("1"), test("2"), test("3"), test("5"), test("8") };
	auto il3 = { test("2"), test("3"), test("5"), test("7"), test("11"), test("13") };


	test_insert(2, il1);
	test_insert(3, il1);
	test_insert(5, il1);

	test_insert(0, il2);
	test_insert(4, il2);
	test_insert(6, il2);

	test_insert(1, il3);
	test_insert(3, il3);
	test_insert(5, il3);

}


TEST(FIXED_VECTOR, push_back) {

	vecl::fixed_vector<test, 16> a;
	auto size = a.size();
	test example{ "YO" };

	a.push_back(example);
	ASSERT_EQ(a.size(), size + 1);
	ASSERT_FALSE(a.empty());
	ASSERT_EQ(a[0].id, example.id);


	a.push_back(std::move(example));
	ASSERT_EQ(a.size(), size + 2);
	ASSERT_FALSE(a.empty());
	ASSERT_EQ(a[0].id, a[1].id);
	ASSERT_EQ(example.id, "");

}

TEST(FIXED_VECTOR, emplace_back) {

	vecl::fixed_vector<test, 16> a;
	auto size = a.size();
	test example{ "YO" };

	a.emplace_back(example);
	ASSERT_EQ(a.size(), ++size);
	ASSERT_FALSE(a.empty());
	ASSERT_EQ(a[0].id, example.id);


	a.emplace_back(std::move(example));
	ASSERT_EQ(a.size(), ++size);
	ASSERT_FALSE(a.empty());
	ASSERT_EQ(a[0].id, a[1].id);
	ASSERT_EQ(example.id, "");

	test example2;
	a.emplace_back();
	ASSERT_EQ(a.size(), ++size);
	ASSERT_FALSE(a.empty());
	ASSERT_EQ(a.back().id, example2.id);

	a.emplace_back("LAST");
	ASSERT_EQ(a.size(), ++size);
	ASSERT_FALSE(a.empty());
	ASSERT_EQ(a.back().id, "LAST");


}

TEST(FIXED_VECTOR, emplace) {

	vecl::fixed_vector<test, 16> a{ test("0"), test("1"), test("2"), test("3"), test("4"), test("5") };
	auto size = a.size();
	test example{ "YO" };

	auto it = a.emplace(a.begin() + 1, example);
	ASSERT_EQ(a.size(), ++size);
	ASSERT_FALSE(a.empty());
	ASSERT_EQ(it->id, example.id);

	it = a.emplace(a.begin() + 1, std::move(example));
	ASSERT_EQ(a.size(), ++size);
	ASSERT_FALSE(a.empty());
	ASSERT_EQ(it->id, "YO");
	ASSERT_EQ(example.id, "");

	test example2;
	it = a.emplace(a.begin() + 3);
	ASSERT_EQ(a.size(), ++size);
	ASSERT_FALSE(a.empty());
	ASSERT_EQ(it->id, example2.id);

	it = a.emplace(a.begin() + 5, "LAST");
	ASSERT_EQ(a.size(), ++size);
	ASSERT_FALSE(a.empty());
	ASSERT_EQ(it->id, "LAST");
}

TEST(FIXED_VECTOR, erase_single) {

	auto erase = [](auto& a, auto it) {

		a.erase(a.begin() + it);
	};

	auto equal = [](auto& a, auto& b) {
		if (a.size() != b.size()) return false;

		for (auto i = 0; i < a.size(); ++i)
			if (a[i] != b[i])
				return false;
		return true;
	};

	auto test_erase = [&](unsigned int seed) {

		auto il = { test("0"), test("1"), test("2"), test("3"), test("4"), test("5") };
		vecl::fixed_vector<test, 16> a{ il.begin(), il.end() };
		std::vector<test> b{ il.begin(), il.end() };

		srand(seed);

		while (!a.empty())
		{
			auto it = rand() % a.size();

			erase(a, it);
			erase(b, it);
			ASSERT_TRUE(equal(a, b));
		}


	};

	test_erase(1);
	test_erase(2);
	test_erase(3);
	test_erase(4);
	test_erase(5);

}

TEST(FIXED_VECTOR, erase_range) {

	auto erase = [](auto& a, auto first, auto second) {

		a.erase(a.begin() + first, a.begin() + second);
	};

	auto equal = [](auto& a, auto& b) {
		if (a.size() != b.size()) return false;

		for (auto i = 0; i < a.size(); ++i)
			if (a[i] != b[i])
				return false;
		return true;
	};

	auto test_erase = [&](unsigned int seed) {

		auto il = { test("0"), test("1"), test("2"), test("3"), test("4"), test("5") };
		vecl::fixed_vector<test, 16> a{ il.begin(), il.end() };
		std::vector<test> b{ il.begin(), il.end() };

		srand(seed);

		while (a.size() > 1)
		{
			auto f = rand() % a.size();
			auto s = rand() % a.size();

			erase(a, std::min(f, s), std::max(f, s));
			erase(b, std::min(f, s), std::max(f, s));

			ASSERT_TRUE(equal(a, b));
		}
	};

	test_erase(1);
	test_erase(2);
	test_erase(3);
	test_erase(4);
	test_erase(5);

}

TEST(FIXED_VECTOR, erase_remove) {

	auto erase_remove = [](int c, unsigned seed) {

		vecl::fixed_vector<int, 16> a;

		srand(seed);
		for (auto i = 0; i < 16; ++i)
			a.push_back(rand() % 5);

		a.erase(std::remove(a.begin(), a.end(), c), a.end());

		for (auto i : a)
			ASSERT_NE(i, c);
	};

	erase_remove(0, 646);
	erase_remove(1, 213);
	erase_remove(2, 13145);
	erase_remove(3, 12314);
	erase_remove(4, 264);
}


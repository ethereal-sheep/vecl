#include <vecl/java_string.hpp>
#include <gtest/gtest.h>
using namespace vecl;

TEST(JAVA_STRING, construct) {
	
    java_string a("123");
    java_string b("125");
}

TEST(JAVA_STRING, equal) {
	
    java_string a("123");
    java_string b("123");

    ASSERT_EQ(a,b);
}
TEST(JAVA_STRING, assignment) {
	
    java_string a = "123";
    java_string b = a;
    java_string c;
    c = "123";

    ASSERT_EQ(a,b);
    ASSERT_EQ(b,c);

}
TEST(JAVA_STRING, concat_cs) {
	
    const char* str1 = "Hello";
    const char* str2 = " World";
    const char* str3 = "Hello World";

    java_string a = str1;
    auto b = a.concat(str2);
    ASSERT_EQ(b, str3);

    auto c = a + str2;
    ASSERT_EQ(c, str3);

    a += str2;
    ASSERT_EQ(a, str3);
}

TEST(JAVA_STRING, concat_sv) {
	
    auto str1 = "Hello"sv;
    auto str2 = " World"sv;
    auto str3 = "Hello World"sv;

    java_string a = str1;
    auto b = a.concat(str2);

    //b == str3;
    ASSERT_EQ(b, str3);

    auto c = a + str2;
    ASSERT_EQ(c, str3);

    a += str2;
    ASSERT_EQ(a, str3);
}

TEST(JAVA_STRING, concat_str) {

    auto str1 = "Hello"s;
    auto str2 = " World"s;
    auto str3 = "Hello World"s;

    java_string a = str1;
    auto b = a.concat(str2);

    //b == str3;
    ASSERT_EQ(b, str3);

    auto c = a + str2;
    ASSERT_EQ(c, str3);

    a += str2;
    ASSERT_EQ(a, str3);
}

TEST(JAVA_STRING, concat_mix) {

    auto str1 = "Hello";
    auto str2 = " World"sv;
    auto str3 = "Hello World"s;

    java_string a = str1;
    auto b = a.concat(str2);

    ASSERT_EQ(b, str3);

    auto c = a + str2;
    ASSERT_EQ(c, str3);

    a += str2;
    ASSERT_EQ(a, str3);

    auto d = a;
    ASSERT_EQ(d, "Hello World");
    ASSERT_EQ(d, "Hello World"sv);
    ASSERT_EQ(d, "Hello World"s);
    ASSERT_EQ(d, a);
}


TEST(JAVA_STRING, size) {

    auto str1 = "Hello"s;
    auto str2 = " World"s;
    auto str3 = "Hello World"s;

    java_string a = str1;
    ASSERT_EQ(a.size(), str1.size());

    java_string b = str2;
    ASSERT_EQ(b.size(), str2.size());

    java_string c = str3;
    ASSERT_EQ(c.size(), str3.size());

}
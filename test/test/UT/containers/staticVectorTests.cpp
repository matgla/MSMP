#include <gtest/gtest.h>

#include "containers/static_vector.hpp"

namespace containers
{

constexpr int data1 = 1;
constexpr int data2 = 2;
constexpr int data3 = 3;

TEST(StaticVectorTests, returnSize)
{
    static_vector<int, 2> vec;
    EXPECT_EQ(0, vec.size());
    vec.push_back(data1);
    EXPECT_EQ(1, vec.size());
    vec.push_back(data1);
    EXPECT_EQ(2, vec.size());
}

TEST(StaticVectorTests, push_back)
{
    static_vector<int, 2> vec;

    EXPECT_EQ(0, vec.size());

    vec.push_back(data1);
    EXPECT_EQ(1, vec.size());
    EXPECT_EQ(data1, vec.get_last());
}

TEST(StaticVectorTests, push_back_notOverflow)
{
    static_vector<int, 1> vec;

    EXPECT_EQ(0, vec.size());

    vec.push_back(data1);
    EXPECT_EQ(1, vec.size());
    EXPECT_EQ(data1, vec.get_last());

    vec.push_back(data2);
    EXPECT_EQ(1, vec.size());
    EXPECT_EQ(data1, vec.get_last());
}

TEST(StaticVectorTests, pop_back_whenEmpty)
{
    static_vector<int, 2> vec;

    EXPECT_EQ(0, vec.size());

    vec.pop_back();
    EXPECT_EQ(0, vec.size());
    vec.push_back(data1);
    vec.pop_back();
    EXPECT_EQ(0, vec.size());

    vec.pop_back();
    EXPECT_EQ(0, vec.size());
}

TEST(StaticVectorTests, pop_back_returnElement)
{
    static_vector<int, 2> vec;

    EXPECT_EQ(0, vec.size());

    vec.pop_back();
    EXPECT_EQ(0, vec.size());

    vec.push_back(data1);
    vec.push_back(data2);
    EXPECT_EQ(2, vec.size());

    EXPECT_EQ(data2, vec.pop_back());
    EXPECT_EQ(1, vec.size());


    EXPECT_EQ(data1, vec.pop_back());
    EXPECT_EQ(0, vec.size());
}

TEST(StaticVectorTests, getLast_returnNullptrWhenEmpty)
{
    static_vector<int, 1> vec;

    EXPECT_EQ(0, vec.size());
    EXPECT_EQ(0, vec.get_last());
}

TEST(StaticVectorTests, getLast_returnLast)
{
    static_vector<int, 2> vec;

    vec.push_back(data1);
    EXPECT_EQ(data1, vec.get_last());

    vec.push_back(data2);
    EXPECT_EQ(data2, vec.get_last());
}

TEST(StaticVectorTests, getLast_returnLastAfterOverflow)
{
    static_vector<int, 2> vec;

    vec.push_back(data1);
    EXPECT_EQ(data1, vec.get_last());

    vec.push_back(data2);
    EXPECT_EQ(data2, vec.get_last());

    vec.push_back(data3);
    EXPECT_EQ(data2, vec.get_last());
}


} // namespace containers

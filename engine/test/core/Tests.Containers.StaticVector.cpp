#include "starlight/core/containers/StaticVector.hh"

#include <gtest/gtest.h>

using namespace sl;

constexpr u64 defaultCapacity = 10;

TEST(StaticVectorTests, givenEmptyVector_whenGettingSize_shouldReturnZero) {
    EXPECT_EQ(StaticVector<int>{ defaultCapacity }.getSize(), 0);
}

TEST(StaticVectorTests, givenVector_whenGettingCapacity_shouldReturnCorrectValue) {
    EXPECT_EQ(StaticVector<int>{ defaultCapacity }.getCapacity(), defaultCapacity);
}

TEST(StaticVectorTests, givenVector_whenInsertingValue_shouldSucceed) {
    StaticVector<int> v{ defaultCapacity };
    auto value = v.emplace(1337);
    ASSERT_TRUE(value != nullptr);
    EXPECT_EQ(*value, 1337);
    EXPECT_EQ(v.getSize(), 1);
}

TEST(StaticVectorTests, givenVector_whenErasingValue_shouldSucceed) {
    StaticVector<int> v{ defaultCapacity };
    auto value = v.emplace(1337);
    ASSERT_TRUE(value != nullptr);
    ASSERT_EQ(v.getSize(), 1);

    EXPECT_TRUE(v.erase(value));
    EXPECT_EQ(v.getSize(), 0);
}

TEST(StaticVectorTests, givenVector_whenErasingNotExistingValue_shouldFail) {
    int v = 1337;
    EXPECT_FALSE(StaticVector<int>{ defaultCapacity }.erase(&v));
}

TEST(StaticVectorTests, givenVector_whenInsertingValueOverCapacity_shouldFail) {
    EXPECT_EQ(StaticVector<int>{ 0 }.emplace(1337), nullptr);
}

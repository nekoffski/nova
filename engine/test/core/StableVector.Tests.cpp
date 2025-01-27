#include "starlight/core/containers/StableVector.hh"

#include <gtest/gtest.h>

using namespace sl;

constexpr u64 defaultCapacity = 10;

TEST(StableVectorTests, givenEmptyVector_whenGettingSize_shouldReturnZero) {
    EXPECT_EQ(StableVector<int>{ defaultCapacity }.getSize(), 0);
}

TEST(StableVectorTests, givenVector_whenGettingCapacity_shouldReturnCorrectValue) {
    EXPECT_EQ(StableVector<int>{ defaultCapacity }.getCapacity(), defaultCapacity);
}

TEST(StableVectorTests, givenVector_whenInsertingValue_shouldSucceed) {
    StableVector<int> v{ defaultCapacity };
    auto value = v.emplace(1337);
    ASSERT_TRUE(value != nullptr);
    EXPECT_EQ(*value, 1337);
    EXPECT_EQ(v.getSize(), 1);
}

TEST(StableVectorTests, givenVector_whenErasingValue_shouldSucceed) {
    StableVector<int> v{ defaultCapacity };
    auto value = v.emplace(1337);
    ASSERT_TRUE(value != nullptr);
    ASSERT_EQ(v.getSize(), 1);

    EXPECT_TRUE(v.erase(value));
    EXPECT_EQ(v.getSize(), 0);
}

TEST(StableVectorTests, givenVector_whenErasingNotExistingValue_shouldFail) {
    int v = 1337;
    EXPECT_FALSE(StableVector<int>{ defaultCapacity }.erase(&v));
}

TEST(StableVectorTests, givenVector_whenInsertingValueOverCapacity_shouldFail) {
    EXPECT_EQ(StableVector<int>{ 0 }.emplace(1337), nullptr);
}

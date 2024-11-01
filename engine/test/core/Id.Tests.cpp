#include "starlight/core/utils/Id.hh"

#include <gtest/gtest.h>

using namespace sl;

static constexpr Id8::Type value1 = 1;
static constexpr Id8::Type value2 = 7;

TEST(IdTests, givenEmptyId_whenConvertingToBoolean_shouldReturnFalse) {
    Id8 id;

    EXPECT_FALSE(id.hasValue());
}

TEST(IdTests, givenId_whenConvertingToBoolean_shouldReturnTrue) {
    Id8 id(value1);
    EXPECT_TRUE(id.hasValue());
}

TEST(IdTests, givenId_whenGettingValue_shouldReturnCorrectValue) {
    Id8 id(value1);
    ASSERT_TRUE(id.hasValue());
    EXPECT_EQ(value1, *id);
    EXPECT_EQ(value1, id.get());
}

TEST(IdTests, givenId_whenReassigningValue_shouldReturnCorrectValue) {
    Id8 id(value1);
    EXPECT_EQ(value1, *id);
    id = value2;
    EXPECT_EQ(value2, *id);
}

TEST(IdTests, givenId_whenInvalidating_shouldInvalidate) {
    Id8 id(value1);
    ASSERT_TRUE(id.hasValue());
    id.invalidate();
    EXPECT_FALSE(id.hasValue());
}

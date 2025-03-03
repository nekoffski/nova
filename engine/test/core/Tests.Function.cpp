#include "starlight/core/Function.hh"

#include <unordered_map>

#include <gtest/gtest.h>

using namespace sl;

TEST(Function, givenFunction_whenConvertingToValue_shouldBeCalled) {
    bool called = false;
    int x       = lazyEvaluate([&]() -> int {
        called = true;
        return 2;
    });
    EXPECT_EQ(x, 2);
    ASSERT_TRUE(called);
}

TEST(Function, givenFunction_whenNotConvertingToValue_shouldNotBeCalled) {
    bool called = false;
    lazyEvaluate([&]() -> int {
        called = true;
        return 2;
    });
    ASSERT_FALSE(called);
}

TEST(Function, givenUnorderedMap_whenRecordExists_shouldNotEvaluate) {
    std::unordered_map<int, int> map;
    bool called   = false;
    const int key = 1;
    map[key]      = 2;

    const auto [it, inserted] = map.try_emplace(
      key, lazyEvaluate([&]() -> int {
          called = true;
          return 3;
      })
    );
    EXPECT_FALSE(inserted);
    EXPECT_FALSE(called);
    EXPECT_EQ(it->second, 2);
}

TEST(Function, givenUnorderedMap_whenRecordNotExist_shouldEvaluate) {
    std::unordered_map<int, int> map;
    bool called               = false;
    const int key             = 1;
    const auto [it, inserted] = map.try_emplace(
      key, lazyEvaluate([&]() -> int {
          called = true;
          return 3;
      })
    );
    EXPECT_TRUE(inserted);
    EXPECT_TRUE(called);
    EXPECT_EQ(it->second, 3);
}

TEST(Function, givenSingleCaller_whenCreating_shouldBeCalled) {
    bool called = false;
    SingleCaller{ [&] { called = true; } };
    EXPECT_TRUE(called);
}
#include "starlight/core/memory/Memory.hh"

#include <gtest/gtest.h>

using namespace sl;

constexpr int value = 1337;

TEST(MemoryTests, whenCreatingUniqPtr_shouldReturnCorrectPointer) {
    auto ptr = createUniqPtr<int>(value);
    ASSERT_NE(ptr.get(), nullptr);
    ASSERT_EQ(*ptr, value);
}

TEST(MemoryTests, whenCreatingRefPtr_shouldReturnCorrectPointer) {
    auto ptr = createRefPtr<int>(value);
    ASSERT_NE(ptr.get(), nullptr);
    ASSERT_EQ(*ptr, value);
}

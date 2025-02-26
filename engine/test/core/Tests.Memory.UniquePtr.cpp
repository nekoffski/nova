#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace testing;

#include "starlight/core/memory/Allocator.hh"
#include "starlight/core/memory/UniquePtr.hh"

using namespace sl;

template <typename T> struct MockAllocator : public Allocator {
    MOCK_METHOD(void*, allocate, (std::size_t n), (override));
    MOCK_METHOD(void, deallocate, (void* ptr, std::size_t n), (noexcept, override));
};

struct Tester {
    Tester() { ++constructorCalls; }
    virtual ~Tester() { ++destructorCalls; }

    virtual int foo() { return 1; }

    inline static uint16_t constructorCalls = 0;
    inline static uint16_t destructorCalls  = 0;
};

struct Tester2 : public Tester {
    Tester2() { ++constructorCalls; }
    virtual ~Tester2() { ++destructorCalls; }

    int foo() override { return 2; }

    inline static uint16_t constructorCalls = 0;
    inline static uint16_t destructorCalls  = 0;
};

struct UniquePtrTests : testing::Test {
    void SetUp() {
        Tester::constructorCalls  = 0;
        Tester::destructorCalls   = 0;
        Tester2::constructorCalls = 0;
        Tester2::destructorCalls  = 0;

        ON_CALL(allocator, allocate)
          .WillByDefault(Invoke([&](std::size_t s) -> Tester* {
              lastAllocatedChunk = s;
              return static_cast<Tester*>(std::malloc(sizeof(Tester)));
          }));
        ON_CALL(allocator, deallocate)
          .WillByDefault(Invoke([&](void* ptr, std::size_t s) {
              EXPECT_EQ(lastAllocatedChunk, s);
              std::free(static_cast<Tester*>(ptr));
          }));
    }

    std::size_t lastAllocatedChunk = 0;
    MockAllocator<Tester> allocator;
};

TEST_F(UniquePtrTests, whenCreatingFromNulltr_shouldNotHoldValue) {
    UniquePtr<int> x = nullptr;
    EXPECT_FALSE(x);
}

TEST_F(
  UniquePtrTests,
  givenDerivedClassPtr_whenCreatingAndDestroying_shouldCallCorrectDctorAndCtor
) {
    ASSERT_NE(&Tester2::constructorCalls, &Tester::constructorCalls);
    ASSERT_NE(&Tester2::destructorCalls, &Tester::destructorCalls);

    {
        UniquePtr<Tester> ptr = UniquePtr<Tester2>::create();

        EXPECT_EQ(Tester2::constructorCalls, 1);
        EXPECT_EQ(Tester2::destructorCalls, 0);
        EXPECT_EQ(Tester::constructorCalls, 1);
        EXPECT_EQ(Tester::destructorCalls, 0);

        EXPECT_EQ(ptr->foo(), 2);
    }
    EXPECT_EQ(Tester2::constructorCalls, 1);
    EXPECT_EQ(Tester2::destructorCalls, 1);
    EXPECT_EQ(Tester::constructorCalls, 1);
    EXPECT_EQ(Tester::destructorCalls, 1);
}

TEST_F(
  UniquePtrTests, givenPtr_whenCreatingAndDestroying_shouldAllocateAndDeallocate
) {
    EXPECT_CALL(allocator, allocate).Times(1);
    EXPECT_CALL(allocator, deallocate).Times(1);

    { auto ptr = UniquePtr<Tester>::create(&allocator); }
}

TEST_F(
  UniquePtrTests, givenDefaultConstructedPtr_whenGettingHandle_shouldReturnNullptr
) {
    UniquePtr<int> ptr;

    EXPECT_EQ(ptr.get(), nullptr);
    EXPECT_EQ(ptr.getAllocator(), nullptr);
}

TEST_F(UniquePtrTests, whenCreatingPointer_shouldStoreCorrectValue) {
    const int value = 1337;
    auto ptr        = UniquePtr<int>::create(value);

    EXPECT_EQ(*ptr, value);
    EXPECT_EQ(*ptr.get(), value);
}

TEST_F(UniquePtrTests, givenPtr_whenMoving_shouldMoveValueCorrectly) {
    const int value = 1337;
    auto ptr        = UniquePtr<int>::create(value);
    auto ptr2       = std::move(ptr);

    EXPECT_EQ(*ptr2, value);
    EXPECT_EQ(*ptr2.get(), value);
}

TEST_F(UniquePtrTests, givenPtr_whenDestroying_shouldCallDestructorOfObject) {
    EXPECT_CALL(allocator, allocate).Times(1);
    EXPECT_CALL(allocator, deallocate).Times(1);

    ASSERT_EQ(Tester::constructorCalls, 0);
    ASSERT_EQ(Tester::destructorCalls, 0);

    {
        auto ptr = UniquePtr<Tester>::create(&allocator);
        EXPECT_EQ(Tester::constructorCalls, 1);
        EXPECT_EQ(Tester::destructorCalls, 0);
    }

    EXPECT_EQ(Tester::constructorCalls, 1);
    EXPECT_EQ(Tester::destructorCalls, 1);
}

TEST_F(UniquePtrTests, givenPtr_whenAssigning_shouldCallDestructorOfObject) {
    EXPECT_CALL(allocator, allocate).Times(2);
    EXPECT_CALL(allocator, deallocate).Times(2);

    ASSERT_EQ(Tester::constructorCalls, 0);
    ASSERT_EQ(Tester::destructorCalls, 0);

    {
        auto ptr = UniquePtr<Tester>::create(&allocator);
        EXPECT_EQ(Tester::constructorCalls, 1);
        EXPECT_EQ(Tester::destructorCalls, 0);

        ptr = UniquePtr<Tester>::create(&allocator);
        EXPECT_EQ(Tester::constructorCalls, 2);
        EXPECT_EQ(Tester::destructorCalls, 1);
    }

    EXPECT_EQ(Tester::constructorCalls, 2);
    EXPECT_EQ(Tester::destructorCalls, 2);
}

TEST_F(
  UniquePtrTests, givenPtr_whenMoveConstructing_shouldNotDestructNorConstructTwice
) {
    EXPECT_CALL(allocator, allocate).Times(1);
    EXPECT_CALL(allocator, deallocate).Times(1);

    ASSERT_EQ(Tester::constructorCalls, 0);
    ASSERT_EQ(Tester::destructorCalls, 0);

    {
        UniquePtr<Tester> ptr;
        auto ptr2 = UniquePtr<Tester>::create(&allocator);

        ASSERT_EQ(Tester::constructorCalls, 1);
        ASSERT_EQ(Tester::destructorCalls, 0);

        ptr = std::move(ptr2);

        ASSERT_EQ(Tester::constructorCalls, 1);
        ASSERT_EQ(Tester::destructorCalls, 0);
    }

    ASSERT_EQ(Tester::constructorCalls, 1);
    ASSERT_EQ(Tester::destructorCalls, 1);
}

TEST_F(
  UniquePtrTests, givenPtr_whenMovingToAnotherPtr_shouldDestructTwiceAndCreateTwice
) {
    EXPECT_CALL(allocator, allocate).Times(2);
    EXPECT_CALL(allocator, deallocate).Times(2);

    ASSERT_EQ(Tester::constructorCalls, 0);
    ASSERT_EQ(Tester::destructorCalls, 0);

    {
        auto ptr  = UniquePtr<Tester>::create(&allocator);
        auto ptr2 = UniquePtr<Tester>::create(&allocator);

        ASSERT_EQ(Tester::constructorCalls, 2);
        ASSERT_EQ(Tester::destructorCalls, 0);

        ptr = std::move(ptr2);

        ASSERT_EQ(Tester::constructorCalls, 2);
        ASSERT_EQ(Tester::destructorCalls, 1);
    }

    ASSERT_EQ(Tester::constructorCalls, 2);
    ASSERT_EQ(Tester::destructorCalls, 2);
}

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace testing;

#include "starlight/core/memory/SharedPtr.hh"

using namespace sl;

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

struct SharedPtrTests : testing::Test {
    void SetUp() {
        Tester::constructorCalls = 0;
        Tester::destructorCalls  = 0;
    }

    template <typename Tester>
    void expectTesterCalls(u64 constructor, u64 destructor) {
        ASSERT_EQ(Tester::constructorCalls, constructor);
        ASSERT_EQ(Tester::destructorCalls, destructor);
    }
};

TEST_F(SharedPtrTests, whenCreatingEmptySharedPtr_shouldBeEmpty) {
    SharedPtr<int> p;
    EXPECT_FALSE(p);
    EXPECT_TRUE(p.empty());
}

TEST_F(SharedPtrTests, whenCreatingSharedPtrFromNullPtr_shouldBeEmpty) {
    SharedPtr<int> p = nullptr;
    EXPECT_FALSE(p);
    EXPECT_TRUE(p.empty());
}

TEST_F(SharedPtrTests, whenCreatingSharedPtr_shouldStoreTheCorrectValue) {
    auto p = SharedPtr<int>::create(1);
    ASSERT_TRUE(p);
    ASSERT_FALSE(p.empty());
    EXPECT_EQ(*p, 1);
}

TEST_F(SharedPtrTests, whenDestroyingSharedPtr_shouldCallDestructor) {
    expectTesterCalls<Tester>(0u, 0u);
    {
        auto p = SharedPtr<Tester>::create();
        expectTesterCalls<Tester>(1u, 0u);
        EXPECT_EQ(p->foo(), 1);
    }
    expectTesterCalls<Tester>(1u, 1u);
}

TEST_F(SharedPtrTests, whenResettingSharedPtr_shouldCallDestructor) {
    expectTesterCalls<Tester>(0u, 0u);
    {
        auto p = SharedPtr<Tester>::create();
        expectTesterCalls<Tester>(1u, 0u);
        EXPECT_EQ(p->foo(), 1);
        p.reset();
        expectTesterCalls<Tester>(1u, 1u);
    }
    expectTesterCalls<Tester>(1u, 1u);
}

TEST_F(
  SharedPtrTests,
  whenHavingTwoSharedPtr_shouldExistResourceWhenAllSharedPtrsAreDestroyed
) {
    expectTesterCalls<Tester>(0u, 0u);
    auto p = SharedPtr<Tester>::create();
    expectTesterCalls<Tester>(1u, 0u);
    auto p2 = p.clone();
    expectTesterCalls<Tester>(1u, 0u);
    p.reset();
    expectTesterCalls<Tester>(1u, 0u);
    p2.reset();
    expectTesterCalls<Tester>(1u, 1u);
}

TEST_F(
  SharedPtrTests, whenMovingSharedPtr_shouldExistResourcesOnMovedResourceReset
) {
    expectTesterCalls<Tester>(0u, 0u);
    auto p = SharedPtr<Tester>::create();
    expectTesterCalls<Tester>(1u, 0u);
    SharedPtr<Tester> p2 = std::move(p);
    expectTesterCalls<Tester>(1u, 0u);
    p2.reset();
    expectTesterCalls<Tester>(1u, 1u);
    p.reset();
    expectTesterCalls<Tester>(1u, 1u);
}

TEST_F(SharedPtrTests, whenCreatingView_shouldBeAbleCreatePointerUsingIt) {
    expectTesterCalls<Tester>(0u, 0u);
    auto p = SharedPtr<Tester>::create();
    expectTesterCalls<Tester>(1u, 0u);
    SharedPtr<Tester> view = p;
    expectTesterCalls<Tester>(1u, 0u);
    SharedPtr<Tester> p2 = view;
    expectTesterCalls<Tester>(1u, 0u);
    p.reset();
    expectTesterCalls<Tester>(1u, 0u);
    p2.reset();
    expectTesterCalls<Tester>(1u, 1u);
}

TEST_F(SharedPtrTests, whenCreatingPolymorphicSharedPtr_shouldBeDestroyedProperly) {
    expectTesterCalls<Tester>(0u, 0u);
    expectTesterCalls<Tester2>(0u, 0u);
    SharedPtr<Tester> p = SharedPtr<Tester2>::create();
    expectTesterCalls<Tester>(1u, 0u);
    expectTesterCalls<Tester2>(1u, 0u);
    auto p2 = p.clone();
    expectTesterCalls<Tester>(1u, 0u);
    expectTesterCalls<Tester2>(1u, 0u);
    p.reset();
    expectTesterCalls<Tester>(1u, 0u);
    expectTesterCalls<Tester2>(1u, 0u);
    p2.reset();
    expectTesterCalls<Tester>(1u, 1u);
    expectTesterCalls<Tester2>(1u, 1u);
}

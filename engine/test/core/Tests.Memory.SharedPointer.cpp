#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace testing;

#include "starlight/core/memory/SharedPointer.hh"

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

struct SharedPointerTests : testing::Test {
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

TEST_F(SharedPointerTests, whenCreatingEmptySharedPointer_shouldBeEmpty) {
    SharedPointer<int> p;
    EXPECT_FALSE(p);
    EXPECT_TRUE(p.empty());
}

TEST_F(SharedPointerTests, whenCreatingSharedPointerFromNullPtr_shouldBeEmpty) {
    SharedPointer<int> p = nullptr;
    EXPECT_FALSE(p);
    EXPECT_TRUE(p.empty());
}

TEST_F(SharedPointerTests, whenCreatingSharedPointer_shouldStoreTheCorrectValue) {
    auto p = SharedPointer<int>::create(1);
    ASSERT_TRUE(p);
    ASSERT_FALSE(p.empty());
    EXPECT_EQ(*p, 1);
}

TEST_F(SharedPointerTests, whenDestroyingSharedPointer_shouldCallDestructor) {
    expectTesterCalls<Tester>(0u, 0u);
    {
        auto p = SharedPointer<Tester>::create();
        expectTesterCalls<Tester>(1u, 0u);
        EXPECT_EQ(p->foo(), 1);
    }
    expectTesterCalls<Tester>(1u, 1u);
}

TEST_F(SharedPointerTests, whenResettingSharedPointer_shouldCallDestructor) {
    expectTesterCalls<Tester>(0u, 0u);
    {
        auto p = SharedPointer<Tester>::create();
        expectTesterCalls<Tester>(1u, 0u);
        EXPECT_EQ(p->foo(), 1);
        p.reset();
        expectTesterCalls<Tester>(1u, 1u);
    }
    expectTesterCalls<Tester>(1u, 1u);
}

TEST_F(
  SharedPointerTests,
  whenHavingTwoSharedPointer_shouldExistResourceWhenAllSharedPointersAreDestroyed
) {
    expectTesterCalls<Tester>(0u, 0u);
    auto p = SharedPointer<Tester>::create();
    expectTesterCalls<Tester>(1u, 0u);
    auto p2 = p.clone();
    expectTesterCalls<Tester>(1u, 0u);
    p.reset();
    expectTesterCalls<Tester>(1u, 0u);
    p2.reset();
    expectTesterCalls<Tester>(1u, 1u);
}

TEST_F(
  SharedPointerTests,
  whenMovingSharedPointer_shouldExistResourcesOnMovedResourceReset
) {
    expectTesterCalls<Tester>(0u, 0u);
    auto p = SharedPointer<Tester>::create();
    expectTesterCalls<Tester>(1u, 0u);
    SharedPointer<Tester> p2 = std::move(p);
    expectTesterCalls<Tester>(1u, 0u);
    p2.reset();
    expectTesterCalls<Tester>(1u, 1u);
    p.reset();
    expectTesterCalls<Tester>(1u, 1u);
}

TEST_F(SharedPointerTests, whenCreatingView_shouldBeAbleCreatePointerUsingIt) {
    expectTesterCalls<Tester>(0u, 0u);
    auto p = SharedPointer<Tester>::create();
    expectTesterCalls<Tester>(1u, 0u);
    SharedPointer<Tester> view = p;
    expectTesterCalls<Tester>(1u, 0u);
    SharedPointer<Tester> p2 = view;
    expectTesterCalls<Tester>(1u, 0u);
    p.reset();
    expectTesterCalls<Tester>(1u, 0u);
    p2.reset();
    expectTesterCalls<Tester>(1u, 1u);
}

TEST_F(
  SharedPointerTests, whenCreatingPolymorphicSharedPointer_shouldBeDestroyedProperly
) {
    expectTesterCalls<Tester>(0u, 0u);
    expectTesterCalls<Tester2>(0u, 0u);
    SharedPointer<Tester> p = SharedPointer<Tester2>::create();
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

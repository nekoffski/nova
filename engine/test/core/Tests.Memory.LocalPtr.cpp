#include <gtest/gtest.h>

#include "starlight/core/memory/LocalPtr.hh"

using namespace sl;

struct Foo {
    inline static int ctorCalls  = 0;
    inline static int dctorCalls = 0;

    explicit Foo(int x, float y) : x(x), y(y) { ++ctorCalls; }

    ~Foo() { ++dctorCalls; }

    int x;
    float y;

    int getX() const { return x; }
};

struct LocalPtrTests : testing::Test {
    void SetUp() override {
        Foo::ctorCalls  = 0;
        Foo::dctorCalls = 0;
    }
};

TEST_F(LocalPtrTests, givenNotInitializedPointer_whenAccessing_shouldCrash) {
    LocalPtr<Foo> foo;
    ASSERT_DEATH({ foo->x = 1; }, "");
}

TEST_F(LocalPtrTests, givenInitializedPointer_whenAccessing_shouldNotCrash) {
    LocalPtr<Foo> foo(2, 1.0f);

    ASSERT_NO_FATAL_FAILURE({
        EXPECT_EQ(foo->x, 2);
        EXPECT_EQ(foo->y, 1.0f);
    });
}

TEST_F(LocalPtrTests, givenEmptyPointer_whenEmplacing_shouldNotCrash) {
    {
        LocalPtr<Foo> foo;

        ASSERT_EQ(Foo::ctorCalls, 0);
        ASSERT_EQ(Foo::dctorCalls, 0);

        foo.emplace(2, 1.0f);

        ASSERT_EQ(Foo::ctorCalls, 1);
        ASSERT_EQ(Foo::dctorCalls, 0);

        ASSERT_NO_FATAL_FAILURE({
            EXPECT_EQ(foo->x, 2);
            EXPECT_EQ(foo->y, 1.0f);
        });
    }

    ASSERT_EQ(Foo::ctorCalls, 1);
    ASSERT_EQ(Foo::dctorCalls, 1);
}

TEST_F(LocalPtrTests, givenPointer_whenCreating_shouldCallCtor) {
    ASSERT_EQ(Foo::ctorCalls, 0);
    LocalPtr<Foo> foo(2, 1.0f);
    EXPECT_EQ(Foo::ctorCalls, 1);
}

TEST_F(LocalPtrTests, givenPointer_whenEmplacing_shouldCallCtorAndDctor) {
    ASSERT_EQ(Foo::ctorCalls, 0);
    ASSERT_EQ(Foo::dctorCalls, 0);

    LocalPtr<Foo> foo(2, 1.0f);
    EXPECT_EQ(Foo::ctorCalls, 1);

    foo.emplace(1, 0.5f);

    ASSERT_EQ(Foo::ctorCalls, 2);
    ASSERT_EQ(Foo::dctorCalls, 1);
}

TEST_F(LocalPtrTests, givenPointer_whenDestroying_shouldCallDctor) {
    ASSERT_EQ(Foo::dctorCalls, 0);
    { LocalPtr<Foo> foo(2, 1.0f); }
    EXPECT_EQ(Foo::dctorCalls, 1);
}

TEST_F(LocalPtrTests, givenPointer_whenMoving_shouldDestroyObject) {
    ASSERT_EQ(Foo::dctorCalls, 0);
    ASSERT_EQ(Foo::ctorCalls, 0);
    {
        LocalPtr<Foo> foo(2, 1.0f);
        ASSERT_EQ(Foo::ctorCalls, 1);
        foo = LocalPtr<Foo>(2, 1.5f);
        ASSERT_EQ(Foo::ctorCalls, 2);
        ASSERT_EQ(Foo::dctorCalls, 1);
    }
    EXPECT_EQ(Foo::dctorCalls, 2);
}
#include <gtest/gtest.h>

#include "starlight/core/memory/LocalPointer.hh"

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

struct LocalPointerTests : testing::Test {
    void SetUp() override {
        Foo::ctorCalls  = 0;
        Foo::dctorCalls = 0;
    }
};

TEST_F(LocalPointerTests, givenNotInitializedPointer_whenAccessing_shouldCrash) {
    LocalPointer<Foo> foo;
    ASSERT_DEATH({ foo->x = 1; }, "");
}

TEST_F(LocalPointerTests, givenInitializedPointer_whenAccessing_shouldNotCrash) {
    LocalPointer<Foo> foo(2, 1.0f);

    ASSERT_NO_FATAL_FAILURE({
        EXPECT_EQ(foo->x, 2);
        EXPECT_EQ(foo->y, 1.0f);
    });
}

TEST_F(LocalPointerTests, givenEmptyPointer_whenEmplacing_shouldNotCrash) {
    {
        LocalPointer<Foo> foo;

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

TEST_F(LocalPointerTests, givenPointer_whenCreating_shouldCallCtor) {
    ASSERT_EQ(Foo::ctorCalls, 0);
    LocalPointer<Foo> foo(2, 1.0f);
    EXPECT_EQ(Foo::ctorCalls, 1);
}

TEST_F(LocalPointerTests, givenPointer_whenEmplacing_shouldCallCtorAndDctor) {
    ASSERT_EQ(Foo::ctorCalls, 0);
    ASSERT_EQ(Foo::dctorCalls, 0);

    LocalPointer<Foo> foo(2, 1.0f);
    EXPECT_EQ(Foo::ctorCalls, 1);

    foo.emplace(1, 0.5f);

    ASSERT_EQ(Foo::ctorCalls, 2);
    ASSERT_EQ(Foo::dctorCalls, 1);
}

TEST_F(LocalPointerTests, givenPointer_whenDestroying_shouldCallDctor) {
    ASSERT_EQ(Foo::dctorCalls, 0);
    { LocalPointer<Foo> foo(2, 1.0f); }
    EXPECT_EQ(Foo::dctorCalls, 1);
}

TEST_F(LocalPointerTests, givenPointer_whenMoving_shouldDestroyObject) {
    ASSERT_EQ(Foo::dctorCalls, 0);
    ASSERT_EQ(Foo::ctorCalls, 0);
    {
        LocalPointer<Foo> foo(2, 1.0f);
        ASSERT_EQ(Foo::ctorCalls, 1);
        foo = LocalPointer<Foo>(2, 1.5f);
        ASSERT_EQ(Foo::ctorCalls, 2);
        ASSERT_EQ(Foo::dctorCalls, 1);
    }
    EXPECT_EQ(Foo::dctorCalls, 2);
}
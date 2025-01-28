#include <gtest/gtest.h>

#include "starlight/core/Scope.hh"

struct ScopeTests : testing::Test {
    int onCalls = 0;
};

TEST_F(ScopeTests, givenScopeExit_whenExitingScope_shouldCallCallback) {
    {
        ON_SCOPE_EXIT { onCalls++; };
        EXPECT_EQ(onCalls, 0);
    }
    EXPECT_EQ(onCalls, 1);
}

TEST_F(ScopeTests, givenScopeFail_whenNoExceptionsWasThrown_shouldNotCall) {
    {
        ON_SCOPE_FAIL { onCalls++; };
    }
    EXPECT_EQ(onCalls, 0);
}

TEST_F(ScopeTests, givenScopeSuccess_whenNoExceptionsWasThrown_shouldNotCall) {
    {
        ON_SCOPE_SUCCESS { onCalls++; };
        EXPECT_EQ(onCalls, 0);
    }
    EXPECT_EQ(onCalls, 1);
}

TEST_F(ScopeTests, givenScopeFail_whenExceptionsWasThrown_shouldCall) {
    try {
        ON_SCOPE_FAIL { onCalls++; };
        EXPECT_EQ(onCalls, 0);

        throw 1;
    } catch (...) {
    }

    EXPECT_EQ(onCalls, 1);
}

TEST_F(ScopeTests, givenScopeExit_whenExceptionsWasThrown_shouldCall) {
    try {
        ON_SCOPE_EXIT { onCalls++; };
        EXPECT_EQ(onCalls, 0);

        throw 1;
    } catch (...) {
    }

    EXPECT_EQ(onCalls, 1);
}

TEST_F(ScopeTests, givenScopeSuccess_whenExceptionsWasThrown_shouldNotCall) {
    try {
        ON_SCOPE_SUCCESS { onCalls++; };

        throw 1;
    } catch (...) {
    }

    EXPECT_EQ(onCalls, 0);
}

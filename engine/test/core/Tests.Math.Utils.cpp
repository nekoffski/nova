#include <gtest/gtest.h>

#include "starlight/core/math/Utils.hh"

using namespace sl;

TEST(
  UtilsTests, whenSolvingQuadraticEquationWithoutRootInRealNumbers_shouldReturnNone
) {
    EXPECT_FALSE(solveQuadraticEquation(100.0f, 1.0f, 1.0f).has_value());
}

TEST(
  UtilsTests,
  whenSolvingQuadraticEquationWithDeltaEqualZero_shouldReturnTwoSameValues
) {
    auto roots = solveQuadraticEquation(1.0f, 2.0f, 1.0f);

    EXPECT_TRUE(roots.has_value());

    auto [x0, x1] = roots.value();
    EXPECT_EQ(x0, x1);
}

TEST(
  UtilsTests,
  whenSolvingQuadraticEquationWithPositiveDelta_shouldReturnTwoDifferentRoots
) {
    auto roots = solveQuadraticEquation(1.0f, -14.0f, 11.0f);

    EXPECT_TRUE(roots.has_value());

    constexpr float delta = 0.001f;

    auto [x0, x1] = roots.value();
    EXPECT_NEAR(x0, 0.8355, delta);
    EXPECT_NEAR(x1, 13.1644, delta);
}

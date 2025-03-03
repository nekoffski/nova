#include <gtest/gtest.h>

#include "starlight/core/Singleton.hh"

class Singleton : public sl::Singleton<Singleton> {};

TEST(SingletonTests, givenSingleton_whenGettingInstance_shouldReturnValidAddress) {
    Singleton singleton;
    EXPECT_EQ(&singleton, &Singleton::get());
}

TEST(SingletonTests, givenSingleton_getPtrShouldReturnSameAddressAsGet) {
    Singleton singleton;
    EXPECT_EQ(&Singleton::get(), Singleton::getPtr());
}

TEST(SingletonTests, givenSingleton_whenCreatingSecondInstnace_shouldThrow) {
    Singleton singleton;
    ASSERT_DEATH(Singleton{}, "");
}

TEST(
  SingletonTests,
  givenSingleSingletonWithoutInstance_whenGettingInstancePtr_shouldReturnNullptr
) {
    EXPECT_EQ(Singleton::getPtr(), nullptr);
}

TEST(
  SingletonTests, givenSingleSingletonWithoutInstance_whenGettingInstance_shouldCrash
) {
    ASSERT_DEATH(Singleton::get(), "");
}

TEST(
  SingletonTests,
  givenSingleSingleton_whenCheckingIfIsCreated_shouldReturnCorrectValue
) {
    ASSERT_FALSE(Singleton::isCreated());
    Singleton singleton;
    EXPECT_TRUE(Singleton::isCreated());
}

TEST(
  SingletonTests,
  givenSingleSingleton_whenCallingExpectCreated_shouldReturnCorrectValue
) {
    ASSERT_DEATH(sl::expectCreated<Singleton>(), "");
    Singleton singleton;
    EXPECT_NO_FATAL_FAILURE(sl::expectCreated<Singleton>());
}

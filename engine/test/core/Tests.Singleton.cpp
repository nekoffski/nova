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
  givenSingleSingletonWithoutInstance_whenGettingInstance_shouldReturnNullptr
) {
    EXPECT_EQ(Singleton::getPtr(), nullptr);
}

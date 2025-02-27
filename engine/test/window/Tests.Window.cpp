#include "starlight/window/Window.hh"

#include <gtest/gtest.h>

#include "mock/WindowMock.hh"

#include "starlight/event/EventBroker.hh"
#include "starlight/window/Events.hh"

using namespace sl;
using namespace testing;

struct WindowTests : Test {
    UniquePtr<NiceMock<WindowImplMock>> windowImpl =
      UniquePtr<NiceMock<WindowImplMock>>::create();
};

TEST_F(WindowTests, givenWindow_whenCreating_shouldSetOnWindowCloseCallback) {
    EXPECT_CALL(*windowImpl, onWindowCloseCallback(_)).Times(1);
    Window window{ std::move(windowImpl) };
}

struct WindowCallbacksTests : WindowTests {
    void SetUp() override { called = false; }

    EventBroker eventBroker;
    bool called;
};

TEST_F(WindowCallbacksTests, givenWindow_whenWindowCloses_shouldEmitQuitEvent) {
    [[maybe_unused]] auto id = eventBroker.getProxy().pushEventHandler<QuitEvent>(
      [&]([[maybe_unused]] const auto&) { called = true; }
    );

    EXPECT_CALL(*windowImpl, onWindowCloseCallback)
      .Times(1)
      .WillOnce([](Window::Impl::OnWindowCloseCallback callback) { callback(); });

    Window window{ std::move(windowImpl) };

    eventBroker.dispatch();
    EXPECT_TRUE(called);
}

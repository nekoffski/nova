#include "starlight/window/Window.hh"

#include <gtest/gtest.h>

#include "mock/WindowMock.hh"

#include "starlight/event/EventBroker.hh"
#include "starlight/window/Input.hh"

using namespace sl;
using namespace testing;

struct InputTests : Test {
    NiceMock<WindowImplMock> windowImpl;
};

TEST_F(InputTests, givenInput_whenCreating_shouldSetOnKeyCallback) {
    EXPECT_CALL(windowImpl, onKeyCallback(_)).Times(1);
    Input input{ windowImpl };
}

TEST_F(InputTests, givenInput_whenCreating_shouldSetOnMouseCallback) {
    EXPECT_CALL(windowImpl, onMouseCallback(_)).Times(1);
    Input input{ windowImpl };
}

struct InputCallbacksTests : InputTests {
    void SetUp() override { called = false; }

    EventBroker eventBroker;
    bool called;
};

TEST_F(
  InputCallbacksTests, givenInput_whenMouseInteractionDetected_shouldEmitMouseEvent
) {
    EXPECT_CALL(windowImpl, onMouseCallback)
      .Times(1)
      .WillOnce([](Window::Impl::OnMouseCallback callback) {
          callback(MouseAction::press, 1);
      });

    [[maybe_unused]] auto id = eventBroker.getProxy().pushEventHandler<MouseEvent>(
      [&](const MouseEvent& event) {
          EXPECT_EQ(event.action, MouseAction::press);
          EXPECT_EQ(event.button, 1);
          called = true;
      }
    );

    Input input{ windowImpl };

    eventBroker.dispatch();
    EXPECT_TRUE(called);
}

TEST_F(
  InputCallbacksTests, givenInput_whenKeyboardInteractionDetected_shouldEmitKeyEvent
) {
    EXPECT_CALL(windowImpl, onKeyCallback)
      .Times(1)
      .WillOnce([](Window::Impl::OnKeyCallback callback) {
          callback(KeyAction::press, 1);
      });

    [[maybe_unused]] auto id =
      eventBroker.getProxy().pushEventHandler<KeyEvent>([&](const KeyEvent& event) {
          EXPECT_EQ(event.action, KeyAction::press);
          EXPECT_EQ(event.key, 1);
          called = true;
      });

    Input input{ windowImpl };

    eventBroker.dispatch();
    EXPECT_TRUE(called);
}

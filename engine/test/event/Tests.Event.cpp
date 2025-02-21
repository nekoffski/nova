#include "starlight/event/Core.hh"
#include "starlight/event/EventProxy.hh"
#include "starlight/event/EventBroker.hh"
#include "starlight/event/EventHandlerSentinel.hh"

#include <gtest/gtest.h>

using namespace sl;
using namespace testing;

struct TestEvent {
    float x;
    int y;
};

struct EventTests : Test {
    EventBroker broker;
    EventProxy& proxy = broker.getProxy();
};

TEST_F(EventTests, givenNoHandler_whenEmittingEvent_shouldHaveNoConsequences) {
    EXPECT_NO_THROW({
        proxy.emit<TestEvent>(0.0f, 0);
        broker.dispatch();
    });
}

TEST_F(EventTests, givenHandler_whenEmittingEvent_shouldBeCalled) {
    bool called = false;

    [[maybe_unused]] auto id =
      proxy.pushEventHandler<TestEvent>([&]([[maybe_unused]] const TestEvent& ev) {
          called = true;

          EXPECT_EQ(ev.x, 1337.0f);
          EXPECT_EQ(ev.y, 1337);
      });

    proxy.emit<TestEvent>(1337.0f, 1337);
    broker.dispatch();

    EXPECT_TRUE(called);
}

TEST_F(EventTests, givenTwoHandlers_whenEmittingEventWithPropagate_shouldCallBoth) {
    bool called  = false;
    bool called2 = false;

    [[maybe_unused]] auto id = proxy.pushEventHandler<TestEvent>(
      [&]([[maybe_unused]] const TestEvent& ev) { called = true; }
    );

    id = proxy.pushEventHandler<TestEvent>(
      [&]([[maybe_unused]] const TestEvent& ev) { called2 = true; }
    );

    proxy.emit<TestEvent>(1337.0f, 1337);
    broker.dispatch();

    EXPECT_TRUE(called);
    EXPECT_TRUE(called2);
}

TEST_F(EventTests, givenTwoHandlers_whenEmittingEventWithStop_shouldNotCallBoth) {
    bool called  = false;
    bool called2 = false;

    auto id = proxy.pushEventHandler<TestEvent>(
      [&]([[maybe_unused]] const TestEvent& ev, auto&& handled) {
          called = true;
          handled();
      }
    );

    id = proxy.pushEventHandler<TestEvent>(
      [&]([[maybe_unused]] const TestEvent& ev) { called2 = true; }
    );

    proxy.emit<TestEvent>(1337.0f, 1337);
    broker.dispatch();

    EXPECT_TRUE(called);
    EXPECT_FALSE(called2);
}

TEST_F(EventTests, givenTwoHandlers_whenPoppingHandler_shouldNotBeCalled) {
    bool called  = false;
    bool called2 = false;

    auto id = proxy.pushEventHandler<TestEvent>(
      [&]([[maybe_unused]] const TestEvent& ev) { called = true; }
    );

    id = proxy.pushEventHandler<TestEvent>(
      [&]([[maybe_unused]] const TestEvent& ev) { called2 = true; }
    );

    proxy.emit<TestEvent>(1337.0f, 1337);
    broker.dispatch();

    ASSERT_TRUE(called);
    ASSERT_TRUE(called2);

    called  = false;
    called2 = false;

    proxy.popEventHandler(id);
    proxy.emit<TestEvent>(1337.0f, 1337);
    broker.dispatch();

    EXPECT_TRUE(called);
    EXPECT_FALSE(called2);
}

TEST_F(
  EventTests, givenEventHandlerSentinel_whenDestroying_shouldUnregisterHandlers
) {
    bool called  = false;
    bool called2 = false;

    {
        EventHandlerSentinel sentinel{ proxy };

        sentinel.add<TestEvent>([&]([[maybe_unused]] const TestEvent& ev) {
            called = true;
        });

        sentinel.add<TestEvent>([&]([[maybe_unused]] const TestEvent& ev) {
            called2 = true;
        });

        proxy.emit<TestEvent>(1337.0f, 1337);
        broker.dispatch();
    }

    ASSERT_TRUE(called);
    ASSERT_TRUE(called2);

    called  = false;
    called2 = false;

    proxy.emit<TestEvent>(1337.0f, 1337);
    broker.dispatch();

    EXPECT_FALSE(called);
    EXPECT_FALSE(called2);
}

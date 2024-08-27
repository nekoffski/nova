#include "starlight/core/event/Core.hh"
#include "starlight/core/event/EventProxy.hh"
#include "starlight/core/event/EventBroker.hh"

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
      proxy.pushEventHandler<TestEvent>([&](const TestEvent& ev) {
          called = true;

          EXPECT_EQ(ev.x, 1337.0f);
          EXPECT_EQ(ev.y, 1337);

          return EventChainBehaviour::propagate;
      });

    proxy.emit<TestEvent>(1337.0f, 1337);
    broker.dispatch();

    EXPECT_TRUE(called);
}

TEST_F(EventTests, givenTwoHandlers_whenEmittingEventWithPropagate_shouldCallBoth) {
    bool called  = false;
    bool called2 = false;

    auto id = proxy.pushEventHandler<TestEvent>([&](const TestEvent& ev) {
        called = true;
        return EventChainBehaviour::propagate;
    });

    id = proxy.pushEventHandler<TestEvent>([&](const TestEvent& ev) {
        called2 = true;
        return EventChainBehaviour::propagate;
    });

    proxy.emit<TestEvent>(1337.0f, 1337);
    broker.dispatch();

    EXPECT_TRUE(called);
    EXPECT_TRUE(called2);
}

TEST_F(EventTests, givenTwoHandlers_whenEmittingEventWithStop_shouldNotCallBoth) {
    bool called  = false;
    bool called2 = false;

    auto id = proxy.pushEventHandler<TestEvent>([&](const TestEvent& ev) {
        called = true;
        return EventChainBehaviour::stop;
    });

    id = proxy.pushEventHandler<TestEvent>([&](const TestEvent& ev) {
        called2 = true;
        return EventChainBehaviour::propagate;
    });

    proxy.emit<TestEvent>(1337.0f, 1337);
    broker.dispatch();

    EXPECT_TRUE(called);
    EXPECT_FALSE(called2);
}

TEST_F(EventTests, givenTwoHandlers_whenPoppingHandler_shouldNotBeCalled) {
    bool called  = false;
    bool called2 = false;

    auto id = proxy.pushEventHandler<TestEvent>([&](const TestEvent& ev) {
        called = true;
        return EventChainBehaviour::propagate;
    });

    id = proxy.pushEventHandler<TestEvent>([&](const TestEvent& ev) {
        called2 = true;
        return EventChainBehaviour::propagate;
    });

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
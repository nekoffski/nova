#pragma once

#include <typeindex>
#include <unordered_map>

#include "starlight/core/Core.hh"
#include "Core.hh"
#include "EventProxy.hh"
#include "fwd.hh"

namespace sl {

class EventBroker {
    static constexpr u8 defaultEventCapacity = 128;

public:
    explicit EventBroker();

    EventProxy& getProxy();

    void dispatch();

private:
    details::Events m_events;
    details::EventHandlers m_handlers;
    EventProxy m_proxy;
};

}  // namespace sl
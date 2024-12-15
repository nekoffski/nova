#pragma once

#include <vector>

#include "fwd.hh"
#include "Core.hh"
#include "EventProxy.hh"

namespace sl {

class EventHandlerSentinel {
public:
    explicit EventHandlerSentinel(EventProxy& eventProxy);
    ~EventHandlerSentinel();

    template <typename T>
    EventHandlerSentinel& add(std::function<EventChainBehaviour(const T&)>&& handler
    ) {
        const auto id = m_eventProxy.pushEventHandler<T>(std::move(handler));
        m_handlerIds.push_back(id);
        return *this;
    }

private:
    void addHandlerId(const EventHandlerId& id);

    EventProxy& m_eventProxy;
    std::vector<EventHandlerId> m_handlerIds;
};

}  // namespace sl

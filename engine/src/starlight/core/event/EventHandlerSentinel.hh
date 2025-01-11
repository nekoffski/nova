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

    template <typename T, typename Handler>
    EventHandlerSentinel& add(Handler&& handler)
    requires(Callable<Handler, void, const T&> || Callable<Handler, void, const T&, details::HandledCallback>)
    {
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

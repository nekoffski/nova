#include "EventHandlerSentinel.hh"

#include "EventProxy.hh"

namespace sl {

EventHandlerSentinel::EventHandlerSentinel(EventProxy& eventProxy
) : m_eventProxy(eventProxy) {}

EventHandlerSentinel::~EventHandlerSentinel() {
    for (const auto& handlerId : m_handlerIds) {
        log::debug("Popping event handler with id='{}'", handlerId);
        m_eventProxy.popEventHandler(handlerId);
    }
}

void EventHandlerSentinel::addHandlerId(const EventHandlerId& id) {
    m_handlerIds.push_back(id);
}

}  // namespace sl

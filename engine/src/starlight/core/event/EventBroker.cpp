#include "EventBroker.hh"

namespace sl {

EventBroker::EventBroker() : m_proxy(m_events, m_handlers) {
    m_events.reserve(defaultEventCapacity);
}

EventProxy& EventBroker::getProxy() { return m_proxy; }

void EventBroker::dispatch() {
    for (auto& event : std::exchange(m_events, {})) {
        if (auto handlerRecord = m_handlers.find(event->getType());
            handlerRecord == m_handlers.end()) {
            break;
        } else {
            bool handled = false;
            for (auto& handler : handlerRecord->second) {
                handler.callback(*event, [&handled]() -> void { handled = true; });
                if (handled) break;
            }
        }
    }
}

}  // namespace sl

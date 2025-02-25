#include "EventProxy.hh"

#include <algorithm>
#include <ranges>

namespace sl {

EventProxy::EventProxy(details::Events& events, details::EventHandlers& handlers) :
    m_events(events), m_handlers(handlers) {}

void EventProxy::popEventHandler(const EventHandlerId id) {
    const auto condition = [id](const auto& record) -> bool {
        return record.id == id;
    };

    for (auto& chain : m_handlers | std::views::values)
        if (std::erase_if(chain, condition) != 0) return;

    log::warn("Event handler with id='{}' not found", id);
}

EventHandlerId EventProxy::pushEventHandlerImpl(
  const std::type_index& type, details::EventCallback&& wrapper
) {
    // TODO: ensure thread safety
    auto& chain = m_handlers[type];
    chain.emplace_back(std::move(wrapper));
    return chain.back().id;
}

}  // namespace sl

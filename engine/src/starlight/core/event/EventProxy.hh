#pragma once
#include "kc/core/Singleton.hpp"

#include "starlight/core/memory/Memory.hh"
#include "starlight/core/Log.hh"
#include "Core.hh"

namespace sl {

class EventProxy : public kc::core::Singleton<EventProxy> {
public:
    explicit EventProxy(details::Events& events, details::EventHandlers& handlers);

    template <typename T, typename... Args> void emit(Args&&... args) {
        m_events.push_back(
          createOwningPtr<details::EventStorage<T>>(std::forward<Args>(args)...)
        );
    }

    template <typename T>
    [[nodiscard]] EventHandlerId pushEventHandler(
      std::function<void(const T&, details::HandledCallback&&)>&& handler
    ) {
        return pushEventHandlerImpl(
          typeid(T),
          [handler = std::move(handler)](
            details::EventStorageBase& event, details::HandledCallback&& handled
          ) { handler(event.as<T>(), std::move(handled)); }
        );
    }

    template <typename T>
    [[nodiscard]] EventHandlerId pushEventHandler(
      std::function<void(const T&)>&& handler
    ) {
        return pushEventHandlerImpl(
          typeid(T),
          [handler = std::move(handler)](
            details::EventStorageBase& event,
            [[maybe_unused]] details::HandledCallback&& handled
          ) { handler(event.as<T>()); }
        );
    }

    void popEventHandler(const EventHandlerId id);

private:
    EventHandlerId pushEventHandlerImpl(
      const std::type_index& type, details::EventCallback&& wraper
    );

    details::Events& m_events;
    details::EventHandlers& m_handlers;
};

}  // namespace sl

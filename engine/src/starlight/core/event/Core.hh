#pragma once

#include <typeindex>
#include <functional>
#include <vector>

#include "starlight/core/utils/Id.hh"
#include "starlight/core/memory/Memory.hh"

namespace sl {

using EventHandlerId = u32;

namespace details {

struct EventStorageBase {
    virtual void* getEvent()                = 0;
    virtual std::type_index getType() const = 0;

    template <typename T> T& as() { return *static_cast<T*>(getEvent()); }
};

template <typename T> class EventStorage : public EventStorageBase {
public:
    template <typename... Args>
    explicit EventStorage(Args&&... args) : m_event(std::forward<Args>(args)...) {}

    void* getEvent() override { return static_cast<void*>(&m_event); }
    std::type_index getType() const override { return typeid(T); }

private:
    T m_event;
};

using HandledCallback = std::function<void()>;
using EventCallback   = std::function<void(EventStorageBase&, HandledCallback&&)>;
using Events          = std::vector<OwningPtr<EventStorageBase>>;

struct EventHandlerRecord : Identificable<EventHandlerRecord, EventHandlerId> {
    explicit EventHandlerRecord(EventCallback&& callback);

    EventCallback callback;
};

using EventHandlers =
  std::unordered_map<std::type_index, std::vector<EventHandlerRecord>>;

}  // namespace details
}  // namespace sl

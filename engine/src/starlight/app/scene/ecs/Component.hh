#pragma once

#include "starlight/core/Id.hh"

namespace sl {

template <typename T> class Component : public Identificable<Component<T>> {
public:
    template <typename... Args>
    explicit Component(u64 entityId, Args&&... args) :
        m_entityId(entityId), m_data(std::forward<Args>(args)...) {}

    u64 getEntityId() const { return m_entityId; }

    T& data() { return m_data; }
    const T& data() const { return m_data; }

private:
    u64 m_entityId;
    T m_data;
};

}  // namespace sl

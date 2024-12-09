#pragma once

#include <vector>

#include "starlight/core/Core.hh"
#include "starlight/core/utils/FlatMap.hh"

#include "Component.hh"

namespace sl {

struct ComponentContainerBase {
    virtual ~ComponentContainerBase() = default;
};

template <typename T> class ComponentContainer : public ComponentContainerBase {
    using ComponentBuffer              = FlatMap<u64, Component<T>>;
    static constexpr u64 maxComponents = 1024;

public:
    explicit ComponentContainer() : m_components(maxComponents) {}

    template <typename... Args> Component<T>& add(u64 entityId, Args&&... args) {
        // entityId is a key for the map but also Component<T> ctor argument
        auto record =
          m_components.emplace(entityId, entityId, std::forward<Args>(args)...);
        ASSERT(record, "Could not add component");
        return *record;
    }

    bool has(u64 entityId) { return m_components.has(entityId); }

    Component<T>& get(u64 entityId) { return *m_components.get(entityId); }

    template <typename C>
    requires Callable<C, void, Component<T>&>
    void forEach(C&& callback) {
        m_components.forEach([&]([[maybe_unused]] const auto& k, auto& v) {
            callback(v);
        });
    }

    template <typename C>
    requires Callable<C, void, const u64&, Component<T>&>
    void forEach(C&& callback) {
        m_components.forEach(std::move(callback));
    }

private:
    ComponentBuffer m_components;
};

}  // namespace sl

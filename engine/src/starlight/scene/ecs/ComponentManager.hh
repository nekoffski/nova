#pragma once

#include <typeindex>
#include <unordered_map>

#include "starlight/core/memory/Memory.hh"

#include "Component.hh"
#include "ComponentContainer.hh"

namespace sl {

class ComponentManager {
    using ComponentContainers =
      std::unordered_map<std::type_index, OwningPtr<ComponentContainerBase>>;

public:
    template <typename T, typename... Args>
    Component<T>& add(u64 entityId, Args&&... args) {
        return getComponentContainer<T>().add(entityId, std::forward<Args>(args)...);
    }

    template <typename T> bool has(u64 entityId) {
        return getComponentContainer<T>().has(entityId);
    }

    template <typename T> Component<T>& get(u64 entityId) {
        return getComponentContainer<T>().get(entityId);
    }

    template <typename T> ComponentContainer<T>& getComponentContainer() {
        auto& type = typeid(T);
        // we could calculate hash once but iterator version is very long and hard to
        // read, in case of under-performance - rewrite
        if (not m_componentContainers.contains(type)) [[unlikely]] {
            m_componentContainers.insert(
              { type, createOwningPtr<ComponentContainer<T>>() }
            );
        }
        return static_cast<ComponentContainer<T>&>(*m_componentContainers[type]);
    }

private:
    ComponentContainers m_componentContainers;
};

}  // namespace sl

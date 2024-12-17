#pragma once

#include <optional>
#include <vector>

#include "starlight/core/utils/Id.hh"

#include "Component.hh"
#include "ComponentManager.hh"

namespace sl {

class Entity : public Identificable<Entity> {
public:
    explicit Entity(
      ComponentManager& componentManager, std::optional<std::string> name
    );

    template <typename T, typename... Args>
    Component<T>& addComponent(Args&&... args) {
        m_componentTypes.emplace_back(typeid(T));
        return m_componentManager.add<T>(getId(), std::forward<Args>(args)...);
    }

    template <typename T> Component<T>& getComponent() {
        return m_componentManager.get<T>(getId());
    }

    template <typename T> bool hasComponent() {
        return std::find(m_componentTypes.begin(), m_componentTypes.end(), typeid(T))
               != m_componentTypes.end();
    }

    void* getComponent(std::type_index component) {
        return m_componentManager.getComponent(component, getId());
    }

    std::span<const std::type_index> getComponentTypes() const {
        return m_componentTypes;
    }

private:
    ComponentManager& m_componentManager;
    std::vector<std::type_index> m_componentTypes;

public:
    std::string name;
};

}  // namespace sl

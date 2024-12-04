#pragma once

#include <optional>

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
        return m_componentManager.add<T>(getId(), std::forward<Args>(args)...);
    }

    const std::string& getName();

private:
    ComponentManager& m_componentManager;
    std::string m_name;
};

}  // namespace sl

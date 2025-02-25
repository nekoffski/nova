#include "Entity.hh"

#include <fmt/core.h>

namespace sl {

Entity::Entity(ComponentManager& componentManager, std::optional<std::string> name) :
    m_componentManager(componentManager),
    name(name.value_or(fmt::format("Entity_{}", id))) {}

}  // namespace sl

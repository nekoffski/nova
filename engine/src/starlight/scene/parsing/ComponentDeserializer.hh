#pragma once

#include <string>

#include "starlight/core/Json.hh"
#include "starlight/scene/ecs/Entity.hh"

namespace sl {

struct ComponentDeserializer {
    virtual ~ComponentDeserializer() = default;

    virtual std::string getName() const                                        = 0;
    virtual void deserialize(Entity& entity, const nlohmann::json& node) const = 0;
};

}  // namespace sl

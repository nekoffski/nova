#pragma once

#include <string>

#include "starlight/core/utils/Json.hh"
#include "starlight/scene/ecs/Entity.hh"

namespace sl {

struct ComponentDeserializer {
    virtual ~ComponentDeserializer() = default;

    virtual std::string getName() const                                        = 0;
    virtual void deserialize(Entity& entity, const kc::json::Node& node) const = 0;
};

}  // namespace sl

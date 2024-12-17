#pragma once

#include "starlight/scene/parsing/ComponentSerializer.hh"
#include "starlight/scene/parsing/ComponentDeserializer.hh"

#include "starlight/renderer/light/PointLight.hh"

namespace sl {

struct PointLightSerializer : ComponentSerializer<PointLight> {
    std::string getName() const override;
    kc::json::Node serialize(PointLight& component) const override;
};

struct PointLightDeserializer : ComponentDeserializer {
    std::string getName() const override;
    void deserialize(Entity& entity, const kc::json::Node& json) const override;
};

}  // namespace sl

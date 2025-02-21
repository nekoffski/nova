#pragma once

#include "starlight/app/scene/parsing/ComponentSerializer.hh"
#include "starlight/app/scene/parsing/ComponentDeserializer.hh"

#include "starlight/renderer/light/PointLight.hh"

namespace sl {

struct PointLightSerializer : ComponentSerializer<PointLight> {
    std::string getName() const override;
    nlohmann::json serialize(PointLight& component) const override;
};

struct PointLightDeserializer : ComponentDeserializer {
    std::string getName() const override;
    void deserialize(Entity& entity, const nlohmann::json& json) const override;
};

}  // namespace sl

#pragma once

#include "starlight/scene/parsing/ComponentSerializer.hh"
#include "starlight/scene/parsing/ComponentDeserializer.hh"

#include "starlight/renderer/light/DirectionalLight.hh"

namespace sl {

struct DirectionalLightSerializer : ComponentSerializer<DirectionalLight> {
    std::string getName() const override;
    nlohmann::json serialize(DirectionalLight& component) const override;
};

struct DirectionalLightDeserializer : ComponentDeserializer {
    std::string getName() const override;
    void deserialize(Entity& entity, const nlohmann::json& json) const override;
};

}  // namespace sl

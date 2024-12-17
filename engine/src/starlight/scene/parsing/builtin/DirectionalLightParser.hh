#pragma once

#include "starlight/scene/parsing/ComponentSerializer.hh"
#include "starlight/scene/parsing/ComponentDeserializer.hh"

#include "starlight/renderer/light/DirectionalLight.hh"

namespace sl {

struct DirectionalLightSerializer : ComponentSerializer<DirectionalLight> {
    std::string getName() const override;
    kc::json::Node serialize(DirectionalLight& component) const override;
};

struct DirectionalLightDeserializer : ComponentDeserializer {
    std::string getName() const override;
    void deserialize(Entity& entity, const kc::json::Node& json) const override;
};

}  // namespace sl

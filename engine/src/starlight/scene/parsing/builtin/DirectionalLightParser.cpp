#include "DirectionalLightParser.hh"

namespace sl {

std::string DirectionalLightSerializer::getName() const {
    return "DirectionalLight";
}

kc::json::Node DirectionalLightSerializer::serialize(DirectionalLight& component
) const {
    kc::json::Node json;

    addJsonField(json, "color", component.color);
    addJsonField(json, "direction", component.direction);

    return json;
}

std::string DirectionalLightDeserializer::getName() const {
    return "DirectionalLight";
}

void DirectionalLightDeserializer::deserialize(
  Entity& entity, const kc::json::Node& json
) const {
    entity.addComponent<DirectionalLight>(
      getField<Vec4<f32>>(json, "color"), getField<Vec3<f32>>(json, "direction")
    );
}

}  // namespace sl

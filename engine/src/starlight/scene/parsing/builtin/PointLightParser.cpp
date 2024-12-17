#include "PointLightParser.hh"

namespace sl {

std::string PointLightSerializer::getName() const { return "PointLight"; }

kc::json::Node PointLightSerializer::serialize(PointLight& component) const {
    kc::json::Node json;

    const auto& pointLightData = component.data;

    addJsonField(json, "color", pointLightData.color);
    addJsonField(json, "position", pointLightData.position);
    addJsonField(json, "attenuation", pointLightData.attenuationFactors);

    return json;
}

std::string PointLightDeserializer::getName() const { return "PointLight"; }

void PointLightDeserializer::deserialize(Entity& entity, const kc::json::Node& json)
  const {
    entity.addComponent<PointLight>(
      getField<Vec4<f32>>(json, "color"), getField<Vec3<f32>>(json, "position"),
      getField<Vec3<f32>>(json, "attenuation")
    );
}

}  // namespace sl

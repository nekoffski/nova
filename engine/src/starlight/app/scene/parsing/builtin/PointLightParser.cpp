#include "PointLightParser.hh"

namespace sl {

std::string PointLightSerializer::getName() const { return "PointLight"; }

nlohmann::json PointLightSerializer::serialize(PointLight& component) const {
    nlohmann::json json;
    const auto& pointLightData = component.getShaderData();

    json["color"]       = pointLightData.color;
    json["position"]    = pointLightData.position;
    json["attenuation"] = pointLightData.attenuation;

    return json;
}

std::string PointLightDeserializer::getName() const { return "PointLight"; }

void PointLightDeserializer::deserialize(Entity& entity, const nlohmann::json& json)
  const {
    entity.addComponent<PointLight>(
      json.at("color").get<Vec4<f32>>(), json.at("position").get<Vec3<f32>>(),
      json.at("attenuation").get<Vec3<f32>>()
    );
}

}  // namespace sl

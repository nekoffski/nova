#include "DirectionalLightParser.hh"

namespace sl {

std::string DirectionalLightSerializer::getName() const {
    return "DirectionalLight";
}

nlohmann::json DirectionalLightSerializer::serialize(DirectionalLight& component
) const {
    nlohmann::json json;
    json["color"]     = component.color;
    json["direction"] = component.direction;

    return json;
}

std::string DirectionalLightDeserializer::getName() const {
    return "DirectionalLight";
}

void DirectionalLightDeserializer::deserialize(
  Entity& entity, const nlohmann::json& json
) const {
    entity.addComponent<DirectionalLight>(
      json.at("color").get<Vec4<f32>>(), json.at("direction").get<Vec3<f32>>()
    );
}

}  // namespace sl

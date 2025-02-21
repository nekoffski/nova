#include "SceneParser.hh"

#include "builtin/PointLightParser.hh"
#include "builtin/DirectionalLightParser.hh"
#include "builtin/MeshCompositeParser.hh"

namespace sl {

SceneParser::SceneParser() {
    // serializers
    m_sceneSerializer.addSerializer<PointLightSerializer>()
      .addSerializer<DirectionalLightSerializer>()
      .addSerializer<MeshCompositeSerializer>();

    // deserializers
    m_sceneDeserializer.addDeserializer<PointLightDeserializer>()
      .addDeserializer<DirectionalLightDeserializer>()
      .addDeserializer<MeshCompositeDeserializer>();
}

void SceneParser::serialize(
  Scene& scene, const std::string& path, const FileSystem& fs
) {
    m_sceneSerializer.serialize(scene, path, fs);
}

void SceneParser::deserialize(
  Scene& scene, const std::string& path, const FileSystem& fs
) {
    m_sceneDeserializer.deserialize(scene, path, fs);
}

}  // namespace sl

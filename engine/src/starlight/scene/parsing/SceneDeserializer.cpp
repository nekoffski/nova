#include "SceneDeserializer.hh"

#include "starlight/core/utils/Json.hh"

namespace sl {

void SceneDeserializer::deserialize(
  Scene& scene, const std::string& path, const FileSystem& fs
) {
    LOG_DEBUG("Deserializing scene: {}", path);
    ASSERT(fs.isFile(path), "Scene file does not exist");
    auto root = kc::json::loadJson(fs.readFile(path));

    if (root.isMember("skybox")) {
        const auto skybox = root["skybox"].as<std::string>();
        LOG_DEBUG("Found skybox: {}", skybox);
        scene.skybox = sl::SkyboxFactory::get().load(skybox);
    }

    LOG_DEBUG("Processing entitites");
    for (const auto& entityNode : root["entities"]) parseEntity(scene, entityNode);

    LOG_INFO("Scene successfully loaded: {}", path);
}

void SceneDeserializer::parseEntity(Scene& scene, const kc::json::Node& node) {
    const auto entityName = node["name"].as<std::string>();
    LOG_DEBUG("Processing entity: {}", entityName);

    auto& entity           = scene.addEntity(entityName);
    const auto& components = node["components"];

    for (const auto& componentName : components.getMemberNames()) {
        LOG_DEBUG("Processing component: {}", componentName);
        if (auto it = m_deserializers.find(componentName);
            it != m_deserializers.end()) {
            auto deserializer = it->second.get();
            deserializer->deserialize(entity, components[componentName]);
        } else {
            LOG_ERROR("Could not find deserializer for: {}", componentName);
        }
    }
}

}  // namespace sl

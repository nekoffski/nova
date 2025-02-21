#include "SceneDeserializer.hh"

#include "starlight/renderer/factories/SkyboxFactory.hh"

#include "starlight/core/Json.hh"

namespace sl {

void SceneDeserializer::deserialize(
  Scene& scene, const std::string& path, const FileSystem& fs
) {
    log::debug("Deserializing scene: {}", path);
    log::expect(fs.isFile(path), "Scene file does not exist");
    auto root = nlohmann::json::parse(fs.readFile(path));

    if (json::hasField(root, "skybox")) {
        const auto skybox = root["skybox"].get<std::string>();
        log::debug("Found skybox: {}", skybox);
        scene.skybox = sl::SkyboxFactory::get().load(skybox);
    }

    log::debug("Processing entitites");
    for (const auto& entityNode : root["entities"]) parseEntity(scene, entityNode);

    log::info("Scene successfully loaded: {}", path);
}

void SceneDeserializer::parseEntity(Scene& scene, const nlohmann::json& node) {
    const auto entityName = node["name"].get<std::string>();
    log::debug("Processing entity: {}", entityName);

    auto& entity           = scene.addEntity(entityName);
    const auto& components = node["components"];

    for (const auto& [componentName, body] :
         components.get<nlohmann::json::object_t>()) {
        log::debug("Processing component: {}", componentName);
        if (auto it = m_deserializers.find(componentName);
            it != m_deserializers.end()) {
            auto deserializer = it->second.get();
            deserializer->deserialize(entity, components[componentName]);
        } else {
            log::error("Could not find deserializer for: {}", componentName);
        }
    }
}

}  // namespace sl

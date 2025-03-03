#include "SceneSerializer.hh"

#include "starlight/core/Json.hh"
#include "starlight/core/Time.hh"

namespace sl {

void SceneSerializer::serialize(
  Scene& scene, const std::string& path, const FileSystem& fs
) {
    log::debug("Serializing scene: {}", path);
    nlohmann::json root;

    root["ts"] = getTimeString("%Y-%m-%d %H:%M:%S");

    if (scene.skybox) {
        // const auto skyboxName = scene.skybox->getName();
        // log::debug("Saving skybox: {}", skyboxName);
        // root["skybox"] = skyboxName;
    }

    scene.forEachEntity([&](auto& entity) {
        root["entities"].push_back(parseEntity(entity));
    });

    const auto buffer = root.dump();
    fs.writeFile(path, buffer, FileSystem::WritePolicy::override);
    log::info("Scene successfully saved to: {}", path);
}

nlohmann::json SceneSerializer::parseEntity(Entity& entity) {
    nlohmann::json node;
    log::debug("Processing entity: {}", entity.name);
    node["name"] = entity.name;

    for (const auto component : entity.getComponentTypes()) {
        log::debug("Processing component: {}", component.name());
        if (auto it = m_serializers.find(component); it != m_serializers.end()) {
            auto serializer = it->second.get();
            node["components"][serializer->getName()] =
              serializer->serialize(entity.getComponent(component));
        } else {
            log::error("Serializer not found: {}", component.name());
        }
    }

    return node;
}

}  // namespace sl

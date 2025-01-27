#include "SceneSerializer.hh"

#include "starlight/core/Json.hh"
#include "starlight/core/Clock.hh"

namespace sl {

void SceneSerializer::serialize(
  Scene& scene, const std::string& path, const FileSystem& fs
) {
    log::debug("Serializing scene: {}", path);
    kc::json::Node root;

    root["ts"] = sl::Clock::get().formatTime("%Y-%m-%d %H:%M:%S");

    if (scene.skybox) {
        const auto skyboxName = scene.skybox.getName();
        log::debug("Saving skybox: {}", skyboxName);
        root["skybox"] = skyboxName;
    }

    scene.forEachEntity([&](auto& entity) {
        root["entities"].append(parseEntity(entity));
    });

    const auto buffer = kc::json::toString(root);
    fs.writeFile(path, buffer, kc::core::FileSystem::WritePolicy::override);
    log::info("Scene successfully saved to: {}", path);
}

kc::json::Node SceneSerializer::parseEntity(Entity& entity) {
    kc::json::Node node;
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

#pragma once

#include <string>
#include <functional>
#include <unordered_map>

#include "starlight/core/Core.hh"
#include "starlight/core/FileSystem.hh"
#include "starlight/core/Json.hh"

#include "Scene.hh"

namespace sl {

class SceneDeserializer {
    using Callback = std::function<void(Entity*, const kc::json::Node&)>;

public:
    explicit SceneDeserializer(const FileSystem& fileSystem);

    void registerDeserializer(const std::string& componentName, Callback&& callback);
    void deserialize(Scene& scene, const std::string& path) const;

private:
    void parseScene(Scene& scene, const kc::json::Node& root) const;

    std::unordered_map<std::string, Callback> m_componentDeserializers;
    const FileSystem& m_fileSystem;
};

}  // namespace sl
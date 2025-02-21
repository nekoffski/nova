#pragma once

#include "SceneDeserializer.hh"
#include "SceneSerializer.hh"

namespace sl {

class SceneParser {
public:
    explicit SceneParser();

    void serialize(
      Scene& scene, const std::string& path,
      const FileSystem& fs = FileSystem::getDefault()
    );

    // TODO: return optional<Scene> instead of getting it as an input argument
    void deserialize(
      Scene& scene, const std::string& path,
      const FileSystem& fs = FileSystem::getDefault()
    );

private:
    SceneSerializer m_sceneSerializer;
    SceneDeserializer m_sceneDeserializer;
};

}  // namespace sl

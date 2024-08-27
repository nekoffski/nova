#pragma once

#include <string>
#include <optional>
#include <vector>
#include <unordered_map>

#include "starlight/core/FileSystem.hh"
#include "MeshConfig.hh"
#include "MaterialConfig.hh"

namespace sl {

struct ModelConfig {
    inline static const std::string baseModelsPath =
      SL_ASSETS_PATH + std::string("/models");

    static std::optional<ModelConfig> loadOBJ(
      const std::string& name, const std::string& modelsPath = baseModelsPath,
      const FileSystem& fs = fileSystem
    );

    std::vector<MaterialConfig> materials;
    std::unordered_map<std::string, std::vector<MeshConfig3D>> meshes;
};

}  // namespace sl
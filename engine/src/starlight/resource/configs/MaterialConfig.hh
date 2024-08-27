#pragma once

#include <optional>
#include <string>

#include "starlight/core/math/Core.hh"
#include "starlight/core/FileSystem.hh"

#include "starlight/renderer/Material.hh"

namespace sl {

using namespace std::string_literals;

struct MaterialConfig {
    inline static auto defaultDiffuseColor = Vec4<f32>{ 1.0f };
    inline static auto defaultDiffuseMap   = "Internal.Texture.Default"s;
    inline static auto defaultNormalMap    = "Internal.Texture.DefaultNormalMap"s;
    inline static auto defaultSpecularMap  = "Internal.Texture.DefaultSpecularMap"s;
    inline static auto defaultShader       = "Builtin.Shader.Material"s;
    inline static auto defaultShininess    = 32.0f;

    inline static const std::string baseMaterialsPath =
      SL_ASSETS_PATH + std::string("/materials");

    static MaterialConfig createDefault(const std::string& name);

    static std::optional<MaterialConfig> load(
      const std::string& name, const std::string& materialsPath = baseMaterialsPath,
      const FileSystem& fs = fileSystem
    );

    std::string name;

    Vec4<f32> diffuseColor;
    float shininess;
    std::string diffuseMap;
    std::string specularMap;
    std::string normalMap;

    std::string shaderName;
};

}  // namespace sl
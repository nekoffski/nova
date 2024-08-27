#pragma once

#include <optional>
#include <string>

#include "starlight/renderer/Resource.hh"
#include "starlight/core/math/Core.hh"

#include "gpu/Texture.hh"
#include "gpu/Shader.hh"
#include "fwd.hh"

namespace sl {

using namespace std::string_literals;

class Material {
    inline static const std::string baseMaterialsPath =
      SL_ASSETS_PATH + std::string("/materials");
    inline static auto defaultDiffuseColor = Vec4<f32>{ 1.0f };
    inline static auto defaultDiffuseMap   = "Internal.Texture.Default"s;
    inline static auto defaultNormalMap    = "Internal.Texture.DefaultNormalMap"s;
    inline static auto defaultSpecularMap  = "Internal.Texture.DefaultSpecularMap"s;
    inline static auto defaultShader       = "Builtin.Shader.Material"s;
    inline static auto defaultShininess    = 32.0f;

public:
    struct Config {
        static Config createDefault(const std::string& name);

        static std::optional<Config> load(
          const std::string& name, std::string_view materialsPath,
          const FileSystem& fs
        );

        std::string name;

        Vec4<f32> diffuseColor;
        float shininess;
        std::string diffuseMap;
        std::string specularMap;
        std::string normalMap;

        std::string shaderName;
    };

    struct Properties {
        Vec4<f32> diffuseColor;
        ResourceRef<Texture> diffuseMap;
        ResourceRef<Texture> specularMap;
        ResourceRef<Texture> normalMap;
        float shininess;
        std::string name;
    };

    explicit Material(u64 id, const Properties& props, ResourceRef<Shader> shader);
    ~Material();

    bool isTransparent() const;
    void applyUniforms(u64 frameNumber);

    u64 getId() const;
    const std::string& getName() const;
    const Properties& getProperties() const;

    static ResourceRef<Material> load(
      RendererBackend& renderer, const std::string& name,
      std::string_view materialsPath = baseMaterialsPath,
      const FileSystem& fs           = fileSystem
    );
    static ResourceRef<Material> find(const std::string& name);

private:
    u64 m_id;

    Properties m_props;
    ResourceRef<Shader> m_shader;

    u64 m_renderFrameNumber;
    u64 m_instanceId;
};

struct MaterialManager
    : public ResourceManager<Material>,
      public kc::core::Singleton<MaterialManager> {
    ResourceRef<Material> load(
      RendererBackend& renderer, const std::string& name,
      std::string_view materialsPath, const FileSystem& fs
    );
};

}  // namespace sl
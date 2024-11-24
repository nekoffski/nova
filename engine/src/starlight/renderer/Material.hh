#pragma once

#include <optional>
#include <string>
#include <unordered_map>

#include "starlight/core/utils/Resource.hh"
#include "starlight/core/math/Core.hh"
#include "starlight/core/utils/Id.hh"

#include "fwd.hh"
#include "gpu/Shader.hh"
#include "gpu/Texture.hh"

namespace sl {

using namespace std::string_literals;

class Material : public NonMovable, public Identificable<Material> {
    inline static auto defaultDiffuseColor = Vec4<f32>{ 1.0f };
    inline static auto defaultDiffuseMap   = "Internal.Texture.Default"s;
    inline static auto defaultNormalMap    = "Internal.Texture.DefaultNormalMap"s;
    inline static auto defaultSpecularMap  = "Internal.Texture.DefaultSpecularMap"s;
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
    };

    struct Properties {
        Vec4<f32> diffuseColor;
        ResourceRef<Texture> diffuseMap;
        ResourceRef<Texture> specularMap;
        ResourceRef<Texture> normalMap;
        float shininess;
        std::string name;
    };

    explicit Material(const Properties& props);
    ~Material();

    bool isTransparent() const;
    void applyUniforms(
      Shader& shader, CommandBuffer& commandBuffer, u32 imageIndex,
      const u64 renderFrameNumber
    );

    const std::string& getName() const;
    const Properties& getProperties() const;

    static ResourceRef<Material> load(
      const std::string& name, const FileSystem& fs = fileSystem
    );
    static ResourceRef<Material> find(const std::string& name);

private:
    u64 getShaderInstanceId(Shader&);

    Properties m_props;
    u64 m_renderFrameNumber;

    std::vector<Texture*> m_textures;
    std::unordered_map<u64, u32> m_shaderInstanceIds;
};

class MaterialManager
    : public ResourceManager<Material>,
      public kc::core::Singleton<MaterialManager> {
public:
    explicit MaterialManager(const std::string& path);

    ResourceRef<Material> load(const std::string& name, const FileSystem& fs);

private:
    std::string m_materialsPath;
};

}  // namespace sl

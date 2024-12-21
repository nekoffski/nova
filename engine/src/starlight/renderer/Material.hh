#pragma once

#include <optional>
#include <string>
#include <unordered_map>

#include "starlight/core/utils/Resource.hh"
#include "starlight/core/math/Core.hh"
#include "starlight/core/utils/Id.hh"
#include "starlight/core/memory/Memory.hh"

#include "fwd.hh"
#include "gpu/Shader.hh"
#include "gpu/Texture.hh"

namespace sl {

using namespace std::string_literals;

class Material : public NonMovable, public Identificable<Material> {
    inline static auto defaultDiffuseColor = Vec4<f32>{ 1.0f };
    inline static auto defaultShininess    = 32.0f;

public:
    struct Properties {
        static std::optional<Properties> fromFile(
          const std::string& path, const FileSystem& fs
        );

        static Properties createDefault();

        Vec4<f32> diffuseColor;
        ResourceRef<Texture> diffuseMap;
        ResourceRef<Texture> specularMap;
        ResourceRef<Texture> normalMap;
        float shininess;
    };

    explicit Material(const Properties& props);
    ~Material();

    bool isTransparent() const;
    void applyUniforms(
      ResourceRef<Shader> shader, CommandBuffer& commandBuffer, u32 imageIndex,
      const u64 renderFrameNumber
    );

    const Properties& getProperties() const;

private:
    Properties m_props;
    u64 m_renderFrameNumber;

    std::vector<Texture*> m_textures;
    LocalPtr<Shader::Instance> m_instance;
};

class MaterialManager
    : public ResourceManager<Material>,
      public kc::core::Singleton<MaterialManager> {
public:
    explicit MaterialManager(const std::string& path);

    ResourceRef<Material> create(
      const std::string& name,
      const Material::Properties& properties = Material::Properties::createDefault()
    );
    ResourceRef<Material> create(
      const Material::Properties& properties = Material::Properties::createDefault()
    );
    ResourceRef<Material> load(
      const std::string& name, const FileSystem& fs = fileSystem
    );
    ResourceRef<Material> getDefault();

private:
    std::string m_materialsPath;
    ResourceRef<Material> m_defaultMaterial;
};

}  // namespace sl

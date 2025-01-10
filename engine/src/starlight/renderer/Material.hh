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

    struct Textures {
        ResourceRef<Texture> diffuseMap;
        ResourceRef<Texture> specularMap;
        ResourceRef<Texture> normalMap;

        std::vector<Texture*> asArray();
    };

    explicit Material(const Properties& props);
    ~Material();

    bool isTransparent() const;
    void applyUniforms(
      ResourceRef<Shader> shader, CommandBuffer& commandBuffer, u32 imageIndex,
      const u64 renderFrameNumber
    );

    const Textures& getTextures() const;
    void setTextures(const Textures& textures);

    float shininess;
    Vec4<f32> diffuseColor;

private:
    u64 m_renderFrameNumber;

    Textures m_textures;
    LocalPtr<Shader::Instance> m_instance;
};

class MaterialFactory
    : public ResourceFactory<Material>,
      public kc::core::Singleton<MaterialFactory> {
public:
    explicit MaterialFactory(const std::string& path);

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

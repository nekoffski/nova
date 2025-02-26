#pragma once

#include "starlight/core/Factory.hh"
#include "starlight/renderer/gpu/Texture.hh"
#include "starlight/core/Json.hh"

namespace sl {

class TextureFactory : public Factory<TextureFactory, Texture> {
public:
    explicit TextureFactory();

    SharedPtr<Texture> load(
      const std::string& name, Texture::Type textureType,
      const Texture::SamplerProperties& sampler =
        Texture::SamplerProperties::createDefault()
    );

    SharedPtr<Texture> getDefaultDiffuseMap();
    SharedPtr<Texture> getDefaultNormalMap();
    SharedPtr<Texture> getDefaultSpecularMap();

private:
    void createDefaults();

    SharedPtr<Texture> m_defaultDiffuseMap;
    SharedPtr<Texture> m_defaultNormalMap;
    SharedPtr<Texture> m_defaultSpecularMap;
};

void serialize(nlohmann::json& j, const SharedPtr<Texture>& v);
void deserialize(const nlohmann::json& j, SharedPtr<Texture>& v);

}  // namespace sl

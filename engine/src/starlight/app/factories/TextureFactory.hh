#pragma once

#include "starlight/core/Factory.hh"
#include "starlight/renderer/gpu/Texture.hh"
#include "starlight/core/Json.hh"

namespace sl {

class TextureFactory : public Factory<TextureFactory, Texture> {
public:
    explicit TextureFactory();

    SharedPointer<Texture> load(
      const std::string& name, Texture::Type textureType,
      const Texture::SamplerProperties& sampler =
        Texture::SamplerProperties::createDefault()
    );

    SharedPointer<Texture> getDefaultDiffuseMap();
    SharedPointer<Texture> getDefaultNormalMap();
    SharedPointer<Texture> getDefaultSpecularMap();

private:
    void createDefaults();

    SharedPointer<Texture> m_defaultDiffuseMap;
    SharedPointer<Texture> m_defaultNormalMap;
    SharedPointer<Texture> m_defaultSpecularMap;
};

void serialize(nlohmann::json& j, const SharedPointer<Texture>& v);
void deserialize(const nlohmann::json& j, SharedPointer<Texture>& v);

}  // namespace sl

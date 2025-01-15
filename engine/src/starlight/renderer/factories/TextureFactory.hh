#pragma once

#include "starlight/core/utils/Resource.hh"
#include "starlight/renderer/gpu/Device.hh"

namespace sl {

class TextureFactory
    : public ResourceFactory<Texture>,
      public kc::core::Singleton<TextureFactory> {
public:
    constexpr static Texture::PixelWidth defaultPixelColor = 255;

    explicit TextureFactory(const std::string& path, Device& device);

    ResourceRef<Texture> load(
      const std::string& name, Texture::Type textureType,
      const Texture::SamplerProperties& sampler =
        Texture::SamplerProperties::createDefault()
    );

    ResourceRef<Texture> getDefaultDiffuseMap();
    ResourceRef<Texture> getDefaultNormalMap();
    ResourceRef<Texture> getDefaultSpecularMap();

    ResourceRef<Texture> create(
      const std::string& name,
      const Texture::ImageData& image =
        Texture::ImageData::createDefault(defaultPixelColor),
      const Texture::SamplerProperties& sampler =
        Texture::SamplerProperties::createDefault()
    );

    ResourceRef<Texture> create(
      const Texture::ImageData& image =
        Texture::ImageData::createDefault(defaultPixelColor),
      const Texture::SamplerProperties& sampler =
        Texture::SamplerProperties::createDefault()
    );

private:
    void createDefaults();

    ResourceRef<Texture> m_defaultDiffuseMap;
    ResourceRef<Texture> m_defaultNormalMap;
    ResourceRef<Texture> m_defaultSpecularMap;

    const std::string m_texturesPath;
    Device& m_device;
};

}  // namespace sl
#include "TextureFactory.hh"

namespace sl {

ResourceRef<Texture> TextureFactory::load(
  const std::string& name, Texture::Type textureType,
  const Texture::SamplerProperties& sampler
) {
    if (auto resource = find(name); resource) [[unlikely]]
        return resource;

    const auto fullPath = fmt::format("{}/{}", m_texturesPath, name);

    if (auto data = Texture::ImageData::load(fullPath, textureType); data)
        return store(name, m_device.createTexture(*data, sampler));

    log::warn("Could not process texture: {}", fullPath);
    return nullptr;
}

ResourceRef<Texture> TextureFactory::getDefaultDiffuseMap() {
    return m_defaultDiffuseMap;
}

ResourceRef<Texture> TextureFactory::getDefaultNormalMap() {
    return m_defaultNormalMap;
}

ResourceRef<Texture> TextureFactory::getDefaultSpecularMap() {
    return m_defaultSpecularMap;
}

ResourceRef<Texture> TextureFactory::create(
  const std::string& name, const Texture::ImageData& image,
  const Texture::SamplerProperties& sampler

) {
    return store(name, m_device.createTexture(image, sampler));
}

ResourceRef<Texture> TextureFactory::create(
  const Texture::ImageData& image, const Texture::SamplerProperties& sampler
) {
    return store(m_device.createTexture(image, sampler));
}

TextureFactory::TextureFactory(const std::string& path, Device& device) :
    ResourceFactory("Texture"), m_texturesPath(path), m_device(device) {
    createDefaults();
}

void TextureFactory::createDefaults() {
    auto image            = Texture::ImageData::createDefault();
    const auto bufferSize = image.width * image.height * image.channels;
    image.pixels.resize(bufferSize, 255);

    m_defaultSpecularMap = create("DefaultSpecularMap", image);

    for (auto index = 0u; index < bufferSize; index += image.channels) {
        image.pixels[index]     = 128;
        image.pixels[index + 1] = 128;
        image.pixels[index + 2] = 255;
    }
    m_defaultNormalMap = create("DefaultNormalMap", image);

    static constexpr u8 white    = 255u;
    static constexpr u8 black    = 0u;
    static constexpr u32 squares = 8u;

    const auto gridWidth = image.width / squares;

    for (auto y = 0u; y < image.height; ++y) {
        for (auto x = 0u; x < image.width; ++x) {
            const auto index = y * image.width * image.channels + x * image.channels;

            const auto u = x / gridWidth;
            const auto v = y / gridWidth;

            auto color = (u + v) & 1 ? white : black;

            image.pixels[index]     = color;
            image.pixels[index + 1] = color;
            image.pixels[index + 2] = color;
        }
    }
    m_defaultDiffuseMap = create("DefaultDiffuseMap", image);
}

void serialize(nlohmann::json& j, const ResourceRef<Texture>& v) {}

void deserialize(const nlohmann::json& j, ResourceRef<Texture>& v) {
    v = TextureFactory::get().load(j.get<std::string>(), Texture::Type::flat);
}

}  // namespace sl

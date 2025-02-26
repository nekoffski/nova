#include "TextureFactory.hh"

#include <fmt/core.h>
#include <stb.h>

#include "starlight/core/Globals.hh"
#include "starlight/core/Scope.hh"

namespace sl {

static std::optional<Texture::ImageData> loadFlatImageData(
  std::string_view path, Texture::Orientation orientation
) {
    static constexpr int requiredChannels = 4;

    log::trace("Loading image: '{}'", path);

    int width;
    int height;
    int channels;

    stbi_set_flip_vertically_on_load(orientation == Texture::Orientation::vertical);

    const auto pixels =
      stbi_load(path.data(), &width, &height, &channels, requiredChannels);

    if (not pixels) {
        if (const auto error = stbi_failure_reason(); error)
            log::error("Could not load '{}' - '{}'", path, error);
        return {};
    }

    ON_SCOPE_EXIT { stbi_image_free(pixels); };
    bool isTransparent = false;

    if (channels == 4) {
        static constexpr u8 opaqueAlpha = 255;

        for (int i = 3; i < width * height * channels; i += channels) {
            const auto alpha = *(pixels + i);
            if (alpha != opaqueAlpha) {
                isTransparent = true;
                break;
            }
        }
    }

    if (channels != requiredChannels) {
        log::warn(
          "Image '{}' has different channels count than required - {} != {}", path,
          requiredChannels, channels
        );
    }

    auto image     = Texture::ImageData::createDefault();
    image.width    = static_cast<u32>(width);
    image.height   = static_cast<u32>(height);
    image.channels = static_cast<u32>(requiredChannels);
    image.flags = isTransparent ? Texture::Flags::transparent : Texture::Flags::none;

    const auto bufferSize = width * height * requiredChannels;

    image.pixels.reserve(bufferSize);
    std::copy(pixels, pixels + bufferSize, std::back_inserter(image.pixels));

    log::trace(
      "Image loaded: width={}, height={}, channels={}", image.width, image.height,
      image.channels
    );

    return image;
}

static std::optional<Texture::ImageData> loadCubemapData(std::string_view path) {
    // +X, -X, +Y, -Y, +Z, -Z
    log::debug("Loading cube map: {}", path);

    // TODO: assuming jpg for now but implement some enum to make it
    // configurable
    static std::string extension  = "jpg";
    static constexpr u8 cubeFaces = 6u;

    std::array<std::string, cubeFaces> texturePaths = {
        fmt::format("{}_r.{}", path, extension),
        fmt::format("{}_l.{}", path, extension),
        fmt::format("{}_u.{}", path, extension),
        fmt::format("{}_d.{}", path, extension),
        fmt::format("{}_f.{}", path, extension),
        fmt::format("{}_b.{}", path, extension)
    };

    auto data = Texture::ImageData::createDefault();

    data.type  = Texture::Type::cubemap;
    data.flags = Texture::Flags::none;

    u64 offset = 0;

    for (const auto& path : texturePaths) {
        const auto imageData =
          loadFlatImageData(path, Texture::Orientation::horizontal);

        if (not imageData) {
            log::error("Could not load cubemap face: '{}'", path);
            return {};
        }

        const auto chunkSize =
          imageData->width * imageData->height * imageData->channels;

        if (data.pixels.empty()) {
            data.width    = imageData->width;
            data.height   = imageData->height;
            data.channels = imageData->channels;
            data.pixels.resize(chunkSize * cubeFaces, 0u);
        }

        log::expect(
          imageData->width == data.width && imageData->height == data.height
            && imageData->channels == data.channels,
          "Cube map faces have different size"
        );

        std::memcpy(&data.pixels[offset], imageData->pixels.data(), chunkSize);
        offset += chunkSize;
    }
    return data;
}

static std::optional<Texture::ImageData> loadImageData(
  std::string_view path, Texture::Type textureType
) {
    return textureType == Texture::Type::cubemap
             ? loadCubemapData(path)
             : loadFlatImageData(path, Texture::Orientation::vertical);
}

TextureFactory::TextureFactory() {}

SharedPtr<Texture> TextureFactory::load(
  const std::string& name, Texture::Type textureType,
  const Texture::SamplerProperties& sampler
) {
    if (auto resource = find(name); resource) [[unlikely]]
        return resource;

    const auto texturesPath = Globals::get().getConfig().paths.textures;
    const auto fullPath     = fmt::format("{}/{}", texturesPath, name);

    if (auto data = loadImageData(fullPath, textureType); data)
        return save(name, Texture::create(*data, sampler));

    log::warn("Could not process texture: {}", fullPath);
    return nullptr;
}

void TextureFactory::createDefaults() {
    auto image            = Texture::ImageData::createDefault();
    const auto bufferSize = image.width * image.height * image.channels;
    image.pixels.resize(bufferSize, 255);

    m_defaultSpecularMap = save("Default.SpecularMap", Texture::create(image));

    for (auto index = 0u; index < bufferSize; index += image.channels) {
        image.pixels[index]     = 128;
        image.pixels[index + 1] = 128;
        image.pixels[index + 2] = 255;
    }
    m_defaultNormalMap = save("Default.NormalMap", Texture::create(image));

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
    m_defaultDiffuseMap = save("Default.DiffuseMap", Texture::create(image));
}

SharedPtr<Texture> TextureFactory::getDefaultDiffuseMap() {
    return m_defaultDiffuseMap;
}

SharedPtr<Texture> TextureFactory::getDefaultNormalMap() {
    return m_defaultNormalMap;
}

SharedPtr<Texture> TextureFactory::getDefaultSpecularMap() {
    return m_defaultSpecularMap;
}

void serialize(
  [[maybe_unused]] nlohmann::json& j, [[maybe_unused]] const SharedPtr<Texture>& v
) {}

void deserialize(const nlohmann::json& j, SharedPtr<Texture>& v) {
    v = TextureFactory::get().load(j.get<std::string>(), Texture::Type::flat);
}

}  // namespace sl

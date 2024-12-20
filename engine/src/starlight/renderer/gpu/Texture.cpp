#include "Texture.hh"

#include <stb.h>

#ifdef SL_USE_VK
#include "starlight/renderer/gpu/vulkan/VKTexture.hh"
#include "starlight/renderer/gpu/vulkan/VKRendererBackend.hh"
#endif

namespace sl {

struct TextureData {
    static std::optional<TextureData> fromFile(
      std::string_view path, Texture::Type textureType
    );

    Texture::Properties props;
    Texture::Pixels buffer;
};

std::optional<TextureData> loadFlatTextureData(std::string_view path) {
    LOG_DEBUG("Loading texture '{}'", path);

    if (auto imageData = Texture::ImageData::loadFromFile(path); imageData) {
        auto props          = Texture::Properties::createDefault();
        props.width         = imageData->width;
        props.height        = imageData->height;
        props.channels      = imageData->channels;
        props.isTransparent = imageData->isTransparent;
        props.isWritable    = false;

        return TextureData{ props, imageData->pixels };
    } else {
        LOG_ERROR("Could not load image: {}", path);
    }
    return {};
}

std::optional<TextureData> loadCubemapData(std::string_view path) {
    // +X, -X, +Y, -Y, +Z, -Z
    LOG_DEBUG("Loading cube map: {}", path);

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

    auto props = Texture::Properties::createDefault();

    props.type          = Texture::Type::cubemap;
    props.isTransparent = false;
    props.isWritable    = false;

    Texture::Pixels buffer;

    u64 offset = 0;

    for (const auto& path : texturePaths) {
        const auto imageData = Texture::ImageData::loadFromFile(
          path, Texture::ImageData::Orientation::horizontal
        );

        if (not imageData) {
            LOG_ERROR("Could not load cubemap face: '{}'", path);
            return {};
        }

        const auto chunkSize =
          imageData->width * imageData->height * imageData->channels;

        if (buffer.empty()) {
            props.width    = imageData->width;
            props.height   = imageData->height;
            props.channels = imageData->channels;
            buffer.resize(chunkSize * cubeFaces, 0u);
        }

        ASSERT(
          imageData->width == props.width && imageData->height == props.height
            && imageData->channels == props.channels,
          "Cube map faces have different size"
        );

        std::memcpy(&buffer[offset], imageData->pixels.data(), chunkSize);
        offset += chunkSize;
    }
    return TextureData{ props, buffer };
}

std::optional<TextureData> TextureData::fromFile(
  std::string_view path, Texture::Type textureType
) {
    return textureType == Texture::Type::flat
             ? loadFlatTextureData(path)
             : loadCubemapData(path);
}

Texture::Properties Texture::Properties::createDefault(
  u32 width, u32 height, u32 channels
) {
    return Texture::Properties{
        .width         = width,
        .height        = height,
        .channels      = channels,
        .isTransparent = false,
        .isWritable    = false,
        .type          = Texture::Type::flat,
        .minifyFilter  = Filter::linear,
        .magnifyFilter = Filter::linear,
        .uRepeat       = Repeat::repeat,
        .vRepeat       = Repeat::repeat,
        .wRepeat       = Repeat::repeat,
    };
}

ResourceRef<Texture> Texture::create(
  const Properties& props, const Texture::Pixels& pixels
) {
    return TextureManager::get().create(props, pixels);
}

ResourceRef<Texture> Texture::load(const std::string& name, Type textureType) {
    return TextureManager::get().load(name, textureType);
}

ResourceRef<Texture> Texture::find(const std::string& name) {
    return TextureManager::get().find(name);
}

ResourceRef<Texture> Texture::getDefaultDiffuseMap() {
    return TextureManager::get().getDefaultDiffuseMap();
}

ResourceRef<Texture> Texture::getDefaultNormalMap() {
    return TextureManager::get().getDefaultNormalMap();
}

ResourceRef<Texture> Texture::getDefaultSpecularMap() {
    return TextureManager::get().getDefaultSpecularMap();
}

const Texture::Properties& Texture::getProperties() const { return m_props; }

Texture::Texture(const Properties& props) : m_props(props) {}

std::optional<Texture::ImageData> Texture::ImageData::loadFromFile(
  std::string_view path, Orientation orientation
) {
    static constexpr int requiredChannels = 4;

    LOG_TRACE("Loading image: '{}'", path);

    int width;
    int height;
    int channels;

    stbi_set_flip_vertically_on_load(orientation == Orientation::vertical);

    const auto pixels =
      stbi_load(path.data(), &width, &height, &channels, requiredChannels);

    if (not pixels) {
        if (const auto error = stbi_failure_reason(); error)
            LOG_ERROR("Could not load '{}' - '{}'", path, error);
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
        LOG_WARN(
          "Image '{}' has different channels count than required - {} != {}", path,
          requiredChannels, channels
        );
    }

    ImageData image;
    image.width         = static_cast<u32>(width);
    image.height        = static_cast<u32>(height);
    image.channels      = static_cast<u32>(requiredChannels);
    image.isTransparent = isTransparent;

    const auto bufferSize = width * height * requiredChannels;

    image.pixels.resize(bufferSize, 0);
    std::memcpy(image.pixels.data(), pixels, bufferSize);

    LOG_TRACE(
      "Image loaded: width={}, height={}, channels={}", image.width, image.height,
      image.channels
    );

    return image;
}

ResourceRef<Texture> TextureManager::load(
  const std::string& name, Texture::Type textureType
) {
    if (auto resource = find(name); resource) [[unlikely]]
        return resource;

    const auto fullPath = fmt::format("{}/{}", m_texturesPath, name);

    if (auto data = TextureData::fromFile(fullPath, textureType); data)
        return store(name, createTexture(data->props, data->buffer));

    LOG_WARN("Could not process texture: {}", fullPath);
    return nullptr;
}

ResourceRef<Texture> TextureManager::getDefaultDiffuseMap() {
    return m_defaultDiffuseMap;
}

ResourceRef<Texture> TextureManager::getDefaultNormalMap() {
    return m_defaultNormalMap;
}

ResourceRef<Texture> TextureManager::getDefaultSpecularMap() {
    return m_defaultSpecularMap;
}

ResourceRef<Texture> TextureManager::create(
  const std::string& name, const Texture::Properties& props,
  const Texture::Pixels& pixels
) {
    return store(
      name,
      createTexture(
        props,
        pixels.size() != 0
          ? pixels
          : Texture::Pixels(props.width * props.height * props.channels, 255)
      )
    );
}

ResourceRef<Texture> TextureManager::create(
  const Texture::Properties& props, const Texture::Pixels& pixels
) {
    return store(createTexture(
      props,
      pixels.size() != 0
        ? pixels
        : Texture::Pixels(props.width * props.height * props.channels, 255)
    ));
}

TextureManager::TextureManager(const std::string& path, RendererBackend& renderer) :
    ResourceManager("Texture"), m_texturesPath(path), m_renderer(renderer) {
    createDefaults();
}

void TextureManager::createDefaults() {
    auto properties = Texture::Properties::createDefault();
    const auto bufferSize =
      properties.width * properties.height * properties.channels;
    Texture::Pixels buffer(bufferSize, 255);

    m_defaultSpecularMap = create("DefaultSpecularMap", properties, buffer);

    for (auto index = 0u; index < bufferSize; index += properties.channels) {
        buffer[index]     = 0;
        buffer[index + 1] = 255;
        buffer[index + 2] = 0;
    }
    m_defaultNormalMap = create("DefaultNormalMap", properties, buffer);

    static constexpr u8 white    = 255u;
    static constexpr u8 black    = 0u;
    static constexpr u32 squares = 8u;

    const auto gridWidth = properties.width / squares;

    for (auto y = 0u; y < properties.height; ++y) {
        for (auto x = 0u; x < properties.width; ++x) {
            const auto index =
              y * properties.width * properties.channels + x * properties.channels;

            const auto u = x / gridWidth;
            const auto v = y / gridWidth;

            auto color = (u + v) & 1 ? white : black;

            buffer[index]     = color;
            buffer[index + 1] = color;
            buffer[index + 2] = color;
        }
    }
    m_defaultDiffuseMap = create("DefaultDiffuseMap", properties, buffer);
}

OwningPtr<Texture> TextureManager::createTexture(
  const Texture::Properties& props, const Texture::Pixels& pixels
) {
#ifdef SL_USE_VK
    auto& vkRenderer = static_cast<vk::VKRendererBackend&>(m_renderer);

    return createOwningPtr<vk::VKTexture>(
      vkRenderer.getContext(), vkRenderer.getLogicalDevice(), props, pixels
    );
#else
    FATAL_ERROR("Could not find renderer backend implementation");
#endif
}

}  // namespace sl

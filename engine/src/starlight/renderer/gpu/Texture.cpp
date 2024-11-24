#include "Texture.hh"

#include <stb.h>

#ifdef SL_USE_VK
#include "starlight/renderer/gpu/vulkan/VKTexture.hh"
#include "starlight/renderer/gpu/vulkan/VKRendererBackend.hh"
#endif

namespace sl {

struct TextureComponents {
    Texture::Properties props;
    std::vector<u8> buffer;
};

// TODO: rename
static std::optional<TextureComponents> prepareComponents(
  std::string_view name, Texture::Type textureType
) {
    TextureComponents args;

    auto& props = args.props;
    props.type  = textureType;
    props.name  = name;

    if (textureType == Texture::Type::flat) {
        LOG_DEBUG("Loading texture '{}'", name);

        if (auto imageData = Texture::ImageData::loadFromFile(name); imageData) {
            props.width         = imageData->width;
            props.height        = imageData->height;
            props.channels      = imageData->channels;
            props.isTransparent = imageData->isTransparent;
            props.isWritable    = false;
            props.name          = std::string{ name };
            args.buffer         = imageData->pixels;

        } else {
            LOG_TRACE("Could not load image");
            return {};
        }
    } else {  // cubemap
        // +X, -X, +Y, -Y, +Z, -Z
        LOG_DEBUG("Loading cube map: {}", name);

        // TODO: assuming jpg for now but implement some enum to make it configurable
        static std::string extension  = "jpg";
        static constexpr u8 cubeFaces = 6u;

        std::array<std::string, cubeFaces> texturePaths = {
            fmt::format("{}_r.{}", name, extension),
            fmt::format("{}_l.{}", name, extension),
            fmt::format("{}_u.{}", name, extension),
            fmt::format("{}_d.{}", name, extension),
            fmt::format("{}_f.{}", name, extension),
            fmt::format("{}_b.{}", name, extension)
        };

        props.isTransparent = false;
        props.isWritable    = false;

        auto& buffer = args.buffer;

        for (u8 i = 0u; i < cubeFaces; ++i) {
            const auto& path = texturePaths[i];

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

            u64 offset = i * chunkSize;
            std::memcpy(&buffer[offset], imageData->pixels.data(), chunkSize);
        }
    }
    return args;
}

ResourceRef<Texture> Texture::load(const std::string& name, Type textureType) {
    return TextureManager::get().load(name, textureType);
}

ResourceRef<Texture> Texture::find(const std::string& name) {
    return TextureManager::get().find(name);
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
    if (auto resource = find(name); resource) return resource;

    const auto components =
      prepareComponents(fmt::format("{}/{}", m_texturesPath, name), textureType);

    if (not components) {
        LOG_WARN("Could not process texture: {}/{}", m_texturesPath, name);
        return nullptr;
    }

#ifdef SL_USE_VK
    auto& vkRenderer = static_cast<vk::VKRendererBackend&>(m_renderer);

    return store(
      name,
      createOwningPtr<vk::VKTexture>(
        vkRenderer.getContext(), vkRenderer.getLogicalDevice(), components->props,
        components->buffer
      )
    );
#else
    FATAL_ERROR("Could not find renderer backend implementation");
#endif
}

TextureManager::TextureManager(const std::string& path, RendererBackend& renderer) :
    m_texturesPath(path), m_renderer(renderer) {}

}  // namespace sl

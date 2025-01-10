#include "Texture.hh"

#include <stb.h>

#ifdef SL_USE_VK
#include "starlight/renderer/gpu/vulkan/VKTexture.hh"
#include "starlight/renderer/gpu/vulkan/VKRendererBackend.hh"
#endif

namespace sl {

std::optional<Texture::ImageData> loadFlatImageData(
  std::string_view path, Texture::Orientation orientation
) {
    static constexpr int requiredChannels = 4;

    LOG_TRACE("Loading image: '{}'", path);

    int width;
    int height;
    int channels;

    stbi_set_flip_vertically_on_load(orientation == Texture::Orientation::vertical);

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

    auto image     = Texture::ImageData::createDefault();
    image.width    = static_cast<u32>(width);
    image.height   = static_cast<u32>(height);
    image.channels = static_cast<u32>(requiredChannels);
    image.flags = isTransparent ? Texture::Flags::transparent : Texture::Flags::none;

    const auto bufferSize = width * height * requiredChannels;

    image.pixels.reserve(bufferSize);
    std::copy(pixels, pixels + bufferSize, std::back_inserter(image.pixels));

    LOG_TRACE(
      "Image loaded: width={}, height={}, channels={}", image.width, image.height,
      image.channels
    );

    return image;
}

std::optional<Texture::ImageData> loadCubemapData(std::string_view path) {
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

    auto data = Texture::ImageData::createDefault();

    data.type  = Texture::Type::cubemap;
    data.flags = Texture::Flags::none;

    u64 offset = 0;

    for (const auto& path : texturePaths) {
        const auto imageData =
          loadFlatImageData(path, Texture::Orientation::horizontal);

        if (not imageData) {
            LOG_ERROR("Could not load cubemap face: '{}'", path);
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

        ASSERT(
          imageData->width == data.width && imageData->height == data.height
            && imageData->channels == data.channels,
          "Cube map faces have different size"
        );

        std::memcpy(&data.pixels[offset], imageData->pixels.data(), chunkSize);
        offset += chunkSize;
    }
    return data;
}

Texture::ImageData Texture::ImageData::createDefault(PixelWidth pixelColor) {
    return createDefault(defaultWidth, defaultHeight, defaultChannels, pixelColor);
}

Texture::ImageData Texture::ImageData::createDefault(
  u32 width, u32 height, u8 channels, std::optional<PixelWidth> pixelColor
) {
    Pixels pixels;

    if (pixelColor) {
        const auto bufferSize = width * height * channels;
        pixels.resize(bufferSize, 255);
        for (u64 i = 0u; i < bufferSize; ++i) {
            pixels[i]     = *pixelColor;
            pixels[i + 1] = *pixelColor;
            pixels[i + 2] = *pixelColor;
        }
    }

    return ImageData{
        .width    = width,
        .height   = height,
        .channels = channels,
        .flags    = Flags::none,
        .type     = Type::flat,
        .format   = Format::undefined,
        .tiling   = Tiling::optimal,
        .usage    = Usage::transferDest | Usage::transferSrc | Usage::sampled
                 | Usage::colorAttachment,
        .aspect = Aspect::color,
        .pixels = pixels,
    };
}

Texture::SamplerProperties Texture::SamplerProperties::createDefault() {
    return SamplerProperties{
        .minifyFilter  = Filter::linear,
        .magnifyFilter = Filter::linear,
        .uRepeat       = Repeat::repeat,
        .vRepeat       = Repeat::repeat,
        .wRepeat       = Repeat::repeat,
    };
}

std::optional<Texture::ImageData> Texture::ImageData::load(
  std::string_view path, Texture::Type textureType
) {
    return textureType == Texture::Type::cubemap
             ? loadCubemapData(path)
             : loadFlatImageData(path, Texture::Orientation::vertical);
}

const Texture::SamplerProperties& Texture::getSamplerProperties() const {
    return m_samplerProperties;
}
const Texture::ImageData& Texture::getImageData() const { return m_imageData; }

Texture::Texture(
  const ImageData& imageData, const SamplerProperties& samplerProperties
) : m_imageData(imageData), m_samplerProperties(samplerProperties) {}

ResourceRef<Texture> TextureFactory::load(
  const std::string& name, Texture::Type textureType,
  const Texture::SamplerProperties& sampler
) {
    if (auto resource = find(name); resource) [[unlikely]]
        return resource;

    const auto fullPath = fmt::format("{}/{}", m_texturesPath, name);

    if (auto data = Texture::ImageData::load(fullPath, textureType); data)
        return store(name, Texture::create(m_renderer, *data, sampler));

    LOG_WARN("Could not process texture: {}", fullPath);
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
    return store(name, Texture::create(m_renderer, image, sampler));
}

ResourceRef<Texture> TextureFactory::create(
  const Texture::ImageData& image, const Texture::SamplerProperties& sampler
) {
    return store(Texture::create(m_renderer, image, sampler));
}

TextureFactory::TextureFactory(const std::string& path, RendererBackend& renderer) :
    ResourceFactory("Texture"), m_texturesPath(path), m_renderer(renderer) {
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

OwningPtr<Texture> Texture::create(
  RendererBackend& renderer, const Texture::ImageData& image,
  const Texture::SamplerProperties& sampler
) {
#ifdef SL_USE_VK
    auto& vkRenderer = static_cast<vk::VKRendererBackend&>(renderer);
    return createOwningPtr<vk::VKTexture>(
      vkRenderer.getContext(), vkRenderer.getLogicalDevice(), image, sampler
    );
#else
    FATAL_ERROR("Could not find renderer backend implementation");
#endif
}

}  // namespace sl

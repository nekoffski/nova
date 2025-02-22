#include "Texture.hh"

#include <stb.h>

#include "starlight/core/Log.hh"
#include "starlight/core/Scope.hh"

#include "Device.hh"

#ifdef SL_USE_VK
#include "vulkan/VulkanDevice.hh"
#include "vulkan/VulkanTexture.hh"
#endif

namespace sl {

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

SharedPointer<Texture> Texture::create(
  const ImageData& image, const SamplerProperties& sampler
) {
#ifdef SL_USE_VK
    return SharedPointer<vk::VulkanTexture>::create(
      static_cast<vk::VulkanDevice&>(Device::get().getImpl()), image, sampler
    );
#else
    log::panic("GPU API vendor not specified");
#endif
}

const Texture::SamplerProperties& Texture::getSamplerProperties() const {
    return m_samplerProperties;
}
const Texture::ImageData& Texture::getImageData() const { return m_imageData; }

Texture::Texture(
  const ImageData& imageData, const SamplerProperties& samplerProperties
) : m_imageData(imageData), m_samplerProperties(samplerProperties) {}

}  // namespace sl

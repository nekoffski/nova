#pragma once

#include <span>

#include "starlight/core/utils/Resource.hh"
#include "starlight/core/Core.hh"
#include "starlight/core/memory/Memory.hh"

#include "starlight/renderer/gpu/RendererBackend.hh"
#include "starlight/renderer/Core.hh"

namespace sl {

class Texture : public NonMovable, public Identificable<Texture> {
public:
    static constexpr u32 defaultWidth    = 1024;
    static constexpr u32 defaultHeight   = 1024;
    static constexpr u32 defaultChannels = 4;

    using PixelWidth = u8;
    using Pixels     = std::vector<PixelWidth>;

    enum class Type : u8 { flat, cubemap };
    enum class Filter : u8 { nearest, linear };
    enum class Repeat : u8 { repeat, mirroredRepeat, clampToEdge, clampToBorder };
    enum class Orientation : u8 { vertical, horizontal };
    enum class Flags : u8 { none = 0x0, writable = 0x1, transparent = 0x2 };
    enum class Tiling : u8 { optimal = 0x0, linear = 0x1 };
    enum class Usage : u32 {
        transferSrc            = 0x00000001,
        transferDest           = 0x00000002,
        sampled                = 0x00000004,
        storage                = 0x00000008,
        colorAttachment        = 0x00000010,
        depthStencilAttachment = 0x00000020,
        transientAttachment    = 0x00000040,
        inputAttachment        = 0x00000080,
    };
    enum class Aspect : u32 {
        none     = 0x0,
        color    = 0x00000001,
        depth    = 0x00000002,
        stencil  = 0x00000004,
        metadata = 0x00000008,
    };

    struct ImageData {
        static ImageData createDefault();
        static std::optional<ImageData> load(
          std::string_view path, Texture::Type textureType
        );

        u32 width;
        u32 height;
        u8 channels;
        Flags flags;
        Type type;
        Format format;
        Tiling tiling;
        Usage usage;
        Aspect aspect;
        Pixels pixels;
    };

    struct SamplerProperties {
        static SamplerProperties createDefault();

        Filter minifyFilter;
        Filter magnifyFilter;
        Repeat uRepeat;
        Repeat vRepeat;
        Repeat wRepeat;
    };

    virtual ~Texture() = default;

    virtual void resize(u32 width, u32 height)           = 0;
    virtual void write(u32 offset, std::span<u8> pixels) = 0;

    static OwningPtr<Texture> create(
      RendererBackend& renderer, const ImageData& image = ImageData::createDefault(),
      const SamplerProperties& sampler = SamplerProperties::createDefault()
    );

    const SamplerProperties& getSamplerProperties() const;
    const ImageData& getImageData() const;

protected:
    explicit Texture(
      const ImageData& imageData, const SamplerProperties& samplerProperties
    );

    ImageData m_imageData;
    SamplerProperties m_samplerProperties;
};

class TextureManager
    : public ResourceManager<Texture>,
      public kc::core::Singleton<TextureManager> {
public:
    explicit TextureManager(const std::string& path, RendererBackend& renderer);

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
      const Texture::ImageData& image = Texture::ImageData::createDefault(),
      const Texture::SamplerProperties& sampler =
        Texture::SamplerProperties::createDefault()
    );
    ResourceRef<Texture> create(
      const Texture::ImageData& image = Texture::ImageData::createDefault(),
      const Texture::SamplerProperties& sampler =
        Texture::SamplerProperties::createDefault()
    );

private:
    void createDefaults();

    ResourceRef<Texture> m_defaultDiffuseMap;
    ResourceRef<Texture> m_defaultNormalMap;
    ResourceRef<Texture> m_defaultSpecularMap;

    const std::string m_texturesPath;
    RendererBackend& m_renderer;
};

constexpr void enableBitOperations(Texture::Flags);
constexpr void enableBitOperations(Texture::Aspect);
constexpr void enableBitOperations(Texture::Usage);
constexpr void enableBitOperations(Texture::Tiling);

}  // namespace sl

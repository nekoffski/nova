#pragma once

#include <span>

#include "starlight/core/utils/Resource.hh"
#include "starlight/core/Core.hh"
#include "starlight/core/memory/Memory.hh"

#include "starlight/renderer/gpu/RendererBackend.hh"

namespace sl {

class Texture : public NonMovable, public Identificable<Texture> {
public:
    struct ImageData {
        enum class Orientation { vertical, horizontal };

        static std::optional<ImageData> loadFromFile(
          std::string_view path, Orientation orientation = Orientation::vertical
        );

        std::vector<u8> pixels;
        u32 width;
        u32 height;
        u8 channels;
        bool isTransparent;
    };

    enum class Type : u8 { flat, cubemap };
    enum class Use { unknown, diffuseMap, specularMap, normalMap, cubeMap };
    enum class Filter { nearest, linear };
    enum class Repeat { repeat, mirroredRepeat, clampToEdge, clampToBorder };

    struct Properties {
        u32 width;
        u32 height;
        u32 channels;
        bool isTransparent;
        bool isWritable;
        Type type;
        Use use              = Use::unknown;
        Filter minifyFilter  = Filter::linear;
        Filter magnifyFilter = Filter::linear;
        Repeat uRepeat       = Repeat::repeat;
        Repeat vRepeat       = Repeat::repeat;
        Repeat wRepeat       = Repeat::repeat;
    };

    static ResourceRef<Texture> load(const std::string& name, Type textureType);
    static ResourceRef<Texture> find(const std::string& name);

    virtual ~Texture() = default;

    virtual void resize(u32 width, u32 height)           = 0;
    virtual void write(u32 offset, std::span<u8> pixels) = 0;

    const Properties& getProperties() const;

    static ResourceRef<Texture> getDefaultDiffuseMap();
    static ResourceRef<Texture> getDefaultNormalMap();
    static ResourceRef<Texture> getDefaultSpecularMap();

protected:
    explicit Texture(const Properties& props);

    Properties m_props;
};

class TextureManager
    : public ResourceManager<Texture>,
      public kc::core::Singleton<TextureManager> {
public:
    explicit TextureManager(const std::string& path, RendererBackend& renderer);

    ResourceRef<Texture> load(const std::string& name, Texture::Type textureType);

private:
    const std::string m_texturesPath;
    RendererBackend& m_renderer;
};

}  // namespace sl

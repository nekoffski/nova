#pragma once

#include <optional>
#include <string>
#include <unordered_map>

#include "starlight/core/Resource.hh"
#include "starlight/core/math/Core.hh"
#include "starlight/core/Id.hh"
#include "starlight/core/memory/Memory.hh"

#include "fwd.hh"
#include "starlight/renderer/gpu/Shader.hh"
#include "gpu/ShaderDataBinder.hh"
#include "gpu/Texture.hh"

namespace sl {

using namespace std::string_literals;

class Material : public NonMovable, public Identificable<Material> {
    inline static auto defaultDiffuseColor = Vec4<f32>{ 1.0f };
    inline static auto defaultShininess    = 32.0f;

public:
    struct Textures {
        ResourceRef<Texture> diffuse;
        ResourceRef<Texture> specular;
        ResourceRef<Texture> normal;
    };

    struct Properties {
        static Properties createDefault();

        Vec4<f32> diffuseColor;
        Textures textures;
        float shininess;
    };

    explicit Material(const Properties& props);
    ~Material();

    bool isTransparent() const;

    float shininess;
    Vec4<f32> diffuseColor;
    Textures textures;
};

}  // namespace sl

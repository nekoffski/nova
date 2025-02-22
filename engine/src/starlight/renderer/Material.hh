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
public:
    struct Properties {
        SharedPointer<Texture> diffuseMap;
        SharedPointer<Texture> specularMap;
        SharedPointer<Texture> normalMap;
        Vec4<f32> diffuseColor;
        float shininess;
    };

    explicit Material(const Properties& props);
    ~Material();

    bool isTransparent() const;

    SharedPointer<Texture> diffuseMap;
    SharedPointer<Texture> specularMap;
    SharedPointer<Texture> normalMap;
    float shininess;
    Vec4<f32> diffuseColor;
};

}  // namespace sl

#pragma once

#include <optional>
#include <string>
#include <unordered_map>

#include "starlight/core/math/Core.hh"
#include "starlight/core/Id.hh"
#include "starlight/core/memory/Memory.hh"

#include "fwd.hh"
#include "starlight/renderer/gpu/Shader.hh"
#include "gpu/ShaderDataBinder.hh"
#include "gpu/Texture.hh"

namespace sl {

using namespace std::string_literals;

class Material : public NonMovable, public NamedResource<Material, "Material"> {
public:
    struct Properties {
        SharedPtr<Texture> diffuseMap;
        SharedPtr<Texture> specularMap;
        SharedPtr<Texture> normalMap;
        Vec4<f32> diffuseColor;
        float shininess;
    };

    explicit Material(const Properties& props, OptStr name = {});

    bool isTransparent() const;

    SharedPtr<Texture> diffuseMap;
    SharedPtr<Texture> specularMap;
    SharedPtr<Texture> normalMap;
    float shininess;
    Vec4<f32> diffuseColor;
};

}  // namespace sl

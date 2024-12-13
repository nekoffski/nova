#pragma once

#include <span>
#include <vector>

#include "starlight/core/math/Core.hh"

namespace sl {

class PointLight {
public:
    struct LOD {
        f32 distance;
        f32 opacity;
    };

    struct ShaderData {
        alignas(16) Vec4<f32> color;
        alignas(16) Vec3<f32> position;
        alignas(16) Vec3<f32> attenuationFactors;
    };

    explicit PointLight(
      const Vec4<f32>& color              = Vec4<f32>{ 1.0f },
      const Vec3<f32>& position           = Vec3<f32>{ 0.0f, 0.0f, 0.0 },
      const Vec3<f32>& attenuationFactors = Vec3<f32>{ 0.5f, 1.0f, 1.0f }
    );

    std::span<const LOD> getLODs() const;
    void generateLODs();

    std::string toString() const;

    ShaderData data;

private:
    std::vector<LOD> m_lods;
};

}  // namespace sl

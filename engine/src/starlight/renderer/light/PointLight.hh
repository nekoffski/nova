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

    explicit PointLight(
      const Vec4<f32>& position           = Vec4<f32>{ 0.0f, 0.0f, 0.0, 1.0f },
      const Vec4<f32>& color              = Vec4<f32>{ 1.0f },
      const Vec4<f32>& attenuationFactors = Vec4<f32>{ 0.5f, 1.0f, 1.0f, 0.0f }
    );

    Vec4<f32> position;
    Vec4<f32> color;

    const Vec4<f32>& getAttenuationFactors() const;
    std::span<const LOD> getLODs() const;

private:
    void generateLODs();

    Vec4<f32> m_attenuationFactors;
    std::vector<LOD> m_lods;
};

}  // namespace sl

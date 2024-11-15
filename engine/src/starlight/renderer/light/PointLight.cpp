#include "PointLight.hh"

#include <kc/math/Utils.hpp>

#include "starlight/core/Log.hh"

namespace sl {

PointLight::PointLight(
  const Vec4<f32>& position, const Vec4<f32>& color,
  const Vec4<f32>& attenuationFactors
) : position(position), color(color), m_attenuationFactors(attenuationFactors) {
    generateLODs();
}

const Vec4<f32>& PointLight::getAttenuationFactors() const {
    return m_attenuationFactors;
}

std::span<const PointLight::LOD> PointLight::getLODs() const { return m_lods; }

void PointLight::generateLODs() {
    static const std::vector<std::pair<f32, f32>> targetAttenuations = {
        { 1.0f / 0.99f, 1.0f },
        { 1.0f / 0.5f,  0.2f },
        { 1.0f / 0.3f,  0.1f }
    };

    m_lods.clear();
    m_lods.reserve(targetAttenuations.size());

    for (const auto& [attenuation, opacity] : targetAttenuations) {
        const auto roots = kc::math::solveQuadraticEquation(
          m_attenuationFactors.x, m_attenuationFactors.y,
          m_attenuationFactors.z - attenuation
        );

        if (not roots) {
            LOG_ERROR(
              "Could not solve attenuation equation for light: {}",
              m_attenuationFactors
            );
        }

        const auto distance = std::max(roots->first, roots->second);
        m_lods.push_back({ distance, opacity });
    }
}

}  // namespace sl

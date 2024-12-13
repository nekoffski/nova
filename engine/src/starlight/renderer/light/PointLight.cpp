#include "PointLight.hh"

#include <kc/math/Utils.hpp>

#include "starlight/core/Log.hh"

namespace sl {

PointLight::PointLight(
  const Vec4<f32>& color, const Vec3<f32>& position,
  const Vec3<f32>& attenuationFactors
) : data{ color, position, attenuationFactors } {
    generateLODs();
}

std::span<const PointLight::LOD> PointLight::getLODs() const { return m_lods; }

void PointLight::generateLODs() {
    static const std::vector<std::pair<f32, f32>> targetAttenuations = {
        { 1.0f / 0.99f, 0.7f },
        { 1.0f / 0.5f,  0.2f },
        { 1.0f / 0.3f,  0.1f }
    };

    m_lods.clear();
    m_lods.reserve(targetAttenuations.size());

    for (const auto& [attenuation, opacity] : targetAttenuations) {
        const auto roots = kc::math::solveQuadraticEquation(
          data.attenuationFactors.x, data.attenuationFactors.y,
          data.attenuationFactors.z - attenuation
        );

        if (not roots) {
            LOG_ERROR(
              "Could not solve attenuation equation for light: {}",
              data.attenuationFactors
            );
            continue;
        }

        const auto distance = std::max(roots->first, roots->second);
        m_lods.push_back({ distance, opacity });
    }
}

std::string PointLight::toString() const {
    return fmt::format(
      "PointLight[{}b]: color={}, position={}, attenuation={}",
      sizeof(PointLight::ShaderData), data.color, data.position,
      data.attenuationFactors
    );
}

}  // namespace sl

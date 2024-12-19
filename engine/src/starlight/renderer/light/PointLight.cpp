#include "PointLight.hh"

#include <kc/math/Utils.hpp>

#include "starlight/core/Log.hh"

namespace sl {

PointLight::PointLight(
  const Vec4<f32>& color, const Vec3<f32>& position, const Vec3<f32>& attenuation
) :
    m_data{ color, position, attenuation }, color(m_data.color),
    position(m_data.position) {
    generateLODs();
}

const Vec3<f32>& PointLight::getAttenuation() const { return m_data.attenuation; }

void PointLight::setAttenuation(const Vec3<f32>& attenuation) {
    m_data.attenuation = attenuation;
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
          m_data.attenuation.x, m_data.attenuation.y,
          m_data.attenuation.z - attenuation
        );

        if (not roots) {
            LOG_ERROR(
              "Could not solve attenuation equation for light: {}",
              m_data.attenuation
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
      sizeof(PointLight::ShaderData), m_data.color, m_data.position,
      m_data.attenuation
    );
}

const PointLight::ShaderData& PointLight::getShaderData() const { return m_data; }

}  // namespace sl

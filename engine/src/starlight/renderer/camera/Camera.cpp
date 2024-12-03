#include "Camera.hh"

namespace sl {

Camera::ProjectionProperties Camera::ProjectionProperties::createDefault() {
    return ProjectionProperties{ .fov = 45.0f, .nearZ = 0.1f, .farZ = 1000.0f };
}

Camera::Camera(
  const Vec2<u32>& viewport, const ProjectionProperties& projectionProperties
) : m_viewportSize(viewport), m_projectionProperties(projectionProperties) {
    calculateProjectionMatrix();
}

const Mat4<f32>& Camera::getProjectionMatrix() const { return m_projectionMatrix; }

void Camera::onViewportResize(const Vec2<u32>& viewport) {
    m_viewportSize = viewport;
    calculateProjectionMatrix();
}

void Camera::calculateProjectionMatrix() {
    m_projectionMatrix = math::perspective(
      math::radians(m_projectionProperties.fov),
      static_cast<float>(m_viewportSize.w) / static_cast<float>(m_viewportSize.h),
      m_projectionProperties.nearZ, m_projectionProperties.farZ
    );
}

}  // namespace sl

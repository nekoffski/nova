#include "Camera.hh"

#include "starlight/core/event/WindowResized.hh"

namespace sl {

Camera::Camera(
  const Vec2<u32>& viewport, sl::EventProxy& eventProxy,
  const ProjectionProperties& projectionProperties
) :
    m_viewportSize(viewport), m_projectionProperties(projectionProperties),
    m_eventSentinel(eventProxy) {
    m_eventSentinel.add<WindowResized>([&](auto& event) {
        m_viewportSize = event.size;
        calculateProjectionMatrix();
    });
    calculateProjectionMatrix();
}

const Mat4<f32>& Camera::getProjectionMatrix() const { return m_projectionMatrix; }

void Camera::calculateProjectionMatrix() {
    m_projectionMatrix = math::perspective(
      math::radians(m_projectionProperties.fov),
      static_cast<float>(m_viewportSize.w) / static_cast<float>(m_viewportSize.h),
      m_projectionProperties.nearZ, m_projectionProperties.farZ
    );
}

}  // namespace sl

#include "Cameras.hh"

namespace sle {

Cameras::Cameras(const sl::Vec2<sl::u32>& viewport) :
    m_eulerCamera(sl::EulerCamera::Properties{
      .target       = sl::Vec3<sl::f32>{ 0.0f },
      .radius       = 5.0f,
      .viewportSize = viewport,
    }),
    m_activeCamera(&m_eulerCamera) {}

sl::Camera* Cameras::getActive() { return m_activeCamera; }

void Cameras::update(float deltaTime) { m_eulerCamera.update(deltaTime); }

}  // namespace sle

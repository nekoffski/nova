#include "FirstPersonCamera.hh"

#include <glm/gtc/matrix_transform.hpp>

#include "starlight/core/Log.hh"
#include "starlight/core/window/Keys.hh"
#include "starlight/core/window/Input.hh"

namespace sl {

FirstPersonCamera::FirstPersonCamera(const Properties& props) :
    m_position(props.position), m_front(0.0f, 0.0f, 1.0f), m_up(0.0f, 1.0f, 0.0f),
    m_right(1.0f, 0.0f, 0.0f), m_yaw(0.0f), m_pitch(0.0f), m_speed(10.0f),
    m_viewportSize(props.viewportSize) {}

Mat4<f32> FirstPersonCamera::getViewMatrix() const {
    return glm::lookAt(m_position, m_position + m_front, worldUp);
}

Mat4<f32> FirstPersonCamera::getProjectionMatrix() const {
    // TODO: make it configurable
    return glm::perspective(
      glm::radians(45.0f),
      static_cast<float>(m_viewportSize.w) / static_cast<float>(m_viewportSize.h),
      0.1f, 1000.0f
    );
}

Vec3<f32> FirstPersonCamera::getPosition() const { return m_position; }

void FirstPersonCamera::update(float deltaTime) {
    processInput(m_speed * deltaTime);

    m_front = glm::normalize(Vec3<f32>(
      std::cos(m_yaw) * std::cos(m_pitch), std::sin(m_pitch),
      std::sin(m_yaw) * std::cos(m_pitch)
    ));

    m_right = glm::normalize(glm::cross(m_front, worldUp));
    m_up    = glm::normalize(glm::cross(m_right, m_front));
}

void FirstPersonCamera::processInput(const float speed) {
    auto& input = Input::get();

    if (input.isKeyPressed(SL_KEY_W)) m_position += speed * m_front;
    if (input.isKeyPressed(SL_KEY_S)) m_position -= speed * m_front;
    if (input.isKeyPressed(SL_KEY_D)) m_position += speed * m_right;
    if (input.isKeyPressed(SL_KEY_A)) m_position -= speed * m_right;

    const auto mouseDelta = input.getMousePositionDelta() * 0.001f;
    m_pitch -= mouseDelta.y;
    m_yaw += mouseDelta.x;

    if (input.isKeyPressed(SL_KEY_M)) m_speed += 0.5f;
    if (input.isKeyPressed(SL_KEY_N)) m_speed -= 0.5f;
}

}  // namespace sl

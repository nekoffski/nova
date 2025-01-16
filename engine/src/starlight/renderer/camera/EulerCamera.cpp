#include "EulerCamera.hh"

#include "starlight/core/window/Input.hh"
#include "starlight/core/window/Keys.hh"

namespace sl {

EulerCamera::EulerCamera(const Properties& props, sl::EventProxy& eventProxy) :
    Camera(props.viewportSize, eventProxy), m_target(props.target),
    m_radius(props.radius), m_yaw(90.0f), m_pitch(90.0f) {
    recalculateVectors();
    updateViewMatrix();

    m_eventSentinel.add<sl::ScrollEvent>([&](auto& event) { onScroll(event.offset); }
    );
}

Mat4<f32> EulerCamera::getViewMatrix() const { return m_viewMatrix; }

Vec3<f32> EulerCamera::getPosition() const { return m_position; }

void EulerCamera::update(float deltaTime) {
    static constexpr float speed = 50.0f;

    processInput(speed * deltaTime);
    truncateCoefficients();
    recalculateVectors();
    updateViewMatrix();
}

void EulerCamera::onScroll(float offset) {
    static constexpr f32 scrollSpeed = 0.25f;
    m_radius -= scrollSpeed * offset;
}

void EulerCamera::processInput(const float speed) {
    auto& input = Input::get();

    if (input.isKeyPressed(SL_KEY_W)) m_pitch -= speed;
    if (input.isKeyPressed(SL_KEY_S)) m_pitch += speed;
    if (input.isKeyPressed(SL_KEY_D)) m_yaw -= speed;
    if (input.isKeyPressed(SL_KEY_A)) m_yaw += speed;

    if (input.isKeyPressed(SL_KEY_1)) m_radius += speed / 5.0f;
    if (input.isKeyPressed(SL_KEY_2)) m_radius -= speed / 5.0f;

    if (input.isMouseButtonPressed(SL_MOUSE_BUTTON_2)) {
        input.hideCursor();

        const float scaledSpeed = speed / 10.0f;
        const auto delta        = scaledSpeed * input.getMousePositionDelta();

        m_pitch -= delta.y;
        m_yaw += delta.x;
    } else {
        input.showCursor();
    }
}

void EulerCamera::truncateCoefficients() {
    if (m_yaw >= 360.0f) m_yaw = 0.0f;
    if (m_yaw < 0.0f) m_yaw = 360.0f;

    if (m_pitch > 170.0f) m_pitch = 170.0f;
    if (m_pitch < 10.0f) m_pitch = 10.0f;

    static constexpr float maxRadius = 100.0f;

    if (m_radius < 0.0f) m_radius = 0.0f;
    if (m_radius > maxRadius) m_radius = maxRadius;
}

void EulerCamera::recalculateVectors() {
    const static Vec3<f32> worldUp = Vec3<f32>{ 0.0f, 1.0f, 0.0f };

    const auto pitchRadians = glm::radians(m_pitch);
    const auto yawRadians   = glm::radians(m_yaw);

    // clang-format off
    m_position = m_radius * Vec3<f32> {
        std::sin(pitchRadians) * std::cos(yawRadians),
        std::cos(pitchRadians),
        std::sin(pitchRadians) * std::sin(yawRadians)
    };
    // clang-format on

    m_front = glm::normalize(m_target - m_position);
    m_right = glm::cross(m_front, worldUp);
    m_up    = glm::cross(m_right, m_front);
}

void EulerCamera::updateViewMatrix() {
    m_viewMatrix = glm::lookAt(m_position, m_target, m_up);
}

}  // namespace sl
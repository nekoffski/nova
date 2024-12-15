#pragma once

#include "Camera.hh"

#include "starlight/core/math/Core.hh"

namespace sl {

class EulerCamera : public Camera {
public:
    struct Properties {
        Vec3<f32> target;
        float radius;
        Vec2<u32> viewportSize;
    };

    explicit EulerCamera(const Properties& props, sl::EventProxy& eventProxy);

    Mat4<f32> getViewMatrix() const override;
    Vec3<f32> getPosition() const override;

    void update(float deltaTime) override;

    void onScroll(float offset);

private:
    void processInput(const float speed);
    void truncateCoefficients();
    void recalculateVectors();
    void updateViewMatrix();

    Vec3<f32> m_target;
    float m_radius;

    Vec3<f32> m_position;

    Vec3<f32> m_front;
    Vec3<f32> m_right;
    Vec3<f32> m_up;

    float m_yaw;
    float m_pitch;

    Mat4<f32> m_viewMatrix;
};

}  // namespace sl
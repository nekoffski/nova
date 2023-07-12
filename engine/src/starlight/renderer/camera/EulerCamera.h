#pragma once

#include "Camera.h"

#include "starlight/core/math/Glm.h"

namespace sl {

class EulerCamera : public Camera {
   public:
    struct Properties {
        Vec3f target;
        float radius;
    };

    explicit EulerCamera(const Properties& props);

    Mat4f getViewMatrix() const override;
    Mat4f getProjectionMatrix() const override;
    Vec3f getPosition() const override;

    void update(float deltaTime) override;

   private:
    void processInput(const float speed);
    void truncateCoefficients();
    void recalculateVectors();
    void updateViewMatrix();

    Vec3f m_target;
    float m_radius;

    Vec3f m_position;

    Vec3f m_front;
    Vec3f m_right;
    Vec3f m_up;

    float m_yaw;
    float m_pitch;

    Mat4f m_viewMatrix;
};

}  // namespace sl
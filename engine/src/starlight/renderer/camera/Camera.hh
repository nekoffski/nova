#pragma once

#include "starlight/core/Core.hh"
#include "starlight/core/math/Core.hh"
#include "starlight/core/event/EventHandlerSentinel.hh"

namespace sl {

class Camera {
public:
    struct ProjectionProperties {
        static ProjectionProperties createDefault();

        f32 fov;
        f32 nearZ;
        f32 farZ;
    };

    explicit Camera(
      const Vec2<u32>& viewport,
      const ProjectionProperties& projectionProperties =
        ProjectionProperties::createDefault()
    );

    virtual Mat4<f32> getViewMatrix() const = 0;
    virtual Vec3<f32> getPosition() const   = 0;
    virtual void update(float deltaTime)    = 0;

    const Mat4<f32>& getProjectionMatrix() const;

    void onViewportResize(const Vec2<u32>& viewport);

protected:
    void calculateProjectionMatrix();

    Vec2<u32> m_viewportSize;
    Mat4<f32> m_projectionMatrix;
    ProjectionProperties m_projectionProperties;
};

}  // namespace sl
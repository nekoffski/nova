#pragma once

#include "starlight/core/Core.hh"
#include "starlight/core/math/Core.hh"
#include "starlight/event/EventProxy.hh"
#include "starlight/event/EventHandlerSentinel.hh"

namespace sl {

class Camera {
public:
    struct ProjectionProperties {
        f32 fov   = 45.0f;
        f32 nearZ = 0.1f;
        f32 farZ  = 1000.0f;

        inline static ProjectionProperties getDefault() {
            return ProjectionProperties{};
        }
    };

    explicit Camera(
      const Vec2<u32>& viewport,
      const ProjectionProperties& projectionProperties =
        ProjectionProperties::getDefault()
    );

    virtual Mat4<f32> getViewMatrix() const = 0;
    virtual Vec3<f32> getPosition() const   = 0;
    virtual void update(float deltaTime)    = 0;

    const Mat4<f32>& getProjectionMatrix() const;

protected:
    void calculateProjectionMatrix();

    Vec2<u32> m_viewportSize;
    Mat4<f32> m_projectionMatrix;
    ProjectionProperties m_projectionProperties;
    EventHandlerSentinel m_eventSentinel;
};

}  // namespace sl
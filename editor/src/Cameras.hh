#pragma once

#include <starlight/renderer/camera/Camera.hh>
#include <starlight/renderer/camera/EulerCamera.hh>

namespace sle {

class Cameras {
public:
    explicit Cameras(const sl::Vec2<sl::u32>& viewport);

    sl::Camera* getActive();

    void onViewportResize(const sl::Vec2<sl::u32>& viewport);
    void update(float deltaTime);

private:
    sl::EulerCamera m_eulerCamera;
    sl::Camera* m_activeCamera;
};

}  // namespace sle

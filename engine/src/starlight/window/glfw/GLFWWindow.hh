#pragma once

#include "starlight/core/math/Core.hh"
#include "starlight/core/Config.hh"
#include "starlight/window/Window.hh"

namespace sl::glfw {

class GLFWWindow : public Window::Impl {
    struct Data {
        OnWindowCloseCallback onWindowClose;
        OnKeyCallback onKey;
        OnScrollCallback onScroll;
        OnMouseCallback onMouse;
        OnWindowResizeCallback onWindowResize;

        i32 lastWidth;
        i32 lastHeight;
    };

public:
    explicit GLFWWindow(const Config::Window& config);

    void showCursor() override;
    void hideCursor() override;

    std::string_view getVendor() const override;

    Vec2<u32> getFramebufferSize() const override;
    Vec2<u32> getSize() const override;
    Vec2<f32> getMousePosition() const override;

    bool isKeyPressed(Window::Key keyCode) const override;
    bool isMouseButtonPressed(Window::Button buttonCode) const override;
    void update() override;
    void swapBuffers() override;

    void onKeyCallback(OnKeyCallback) override;
    void onMouseCallback(OnMouseCallback) override;
    void onScrollCallback(OnScrollCallback) override;
    void onWindowCloseCallback(OnWindowCloseCallback) override;
    void onWindowResizeCallback(OnWindowResizeCallback) override;

    void* getHandle() override;

private:
    void* m_windowHandle;
    Data m_windowData;
};

}  // namespace sl::glfw

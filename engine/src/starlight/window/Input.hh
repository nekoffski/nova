#pragma once

#include "starlight/core/Singleton.hh"
#include "Window.hh"

namespace sl {

class Input : public Singleton<Input> {
public:
    using Button = Window::Button;
    using Key    = Window::Key;

    explicit Input(Window::Impl& window);

    Vec2<f32> getMousePosition() const;
    Vec2<f32> getMousePositionDelta() const;

    void showCursor();
    void hideCursor();

    bool isKeyPressed(Key keyCode) const;
    bool isMouseButtonPressed(Button buttonCode) const;

    void update();

private:
    void setCallbacks();
    void calculateMousePositionDelta();

    Window::Impl& m_window;

    Vec2<f32> m_previousMousePosition;
    Vec2<f32> m_mousePositionDelta;
};

}  // namespace sl

#pragma once

#include "WindowImpl.hh"
#include "starlight/core/Singleton.hh"

namespace sl {

class Input : public Singleton<Input> {
public:
    using Button = WindowImpl::Button;
    using Key    = WindowImpl::Key;

    explicit Input(WindowImpl& windowImpl);

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

    WindowImpl& m_windowImpl;

    Vec2<f32> m_previousMousePosition;
    Vec2<f32> m_mousePositionDelta;
};

}  // namespace sl

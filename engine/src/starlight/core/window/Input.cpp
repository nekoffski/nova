#include "Input.hh"

#include "starlight/core/event/EventProxy.hh"
#include "starlight/core/event/Input.hh"

namespace sl {

Input::Input(WindowImpl& windowImpl) :
    m_windowImpl(windowImpl),
    m_previousMousePosition(m_windowImpl.getMousePosition()) {
    setCallbacks();
}

void Input::setCallbacks() {
    m_windowImpl.onKeyCallback([](KeyAction action, int key) {
        KeyEvent event{ action, key };
        log::trace("Detected key action, emitting event: {}", event);
        EventProxy::get().emit<KeyEvent>(event);
    });

    m_windowImpl.onScrollCallback([](float offset) {
        ScrollEvent event{ offset };
        log::trace("Detected scroll, emitting event: {}", event);
        EventProxy::get().emit<ScrollEvent>(event);
    });

    m_windowImpl.onMouseCallback([](MouseAction action, int button) {
        MouseEvent event{ action, button };
        log::trace("Detected mouse action, emitting event: {}", event);
        EventProxy::get().emit<MouseEvent>(event);
    });
}

Vec2<f32> Input::getMousePosition() const { return m_windowImpl.getMousePosition(); }

Vec2<f32> Input::getMousePositionDelta() const { return m_mousePositionDelta; }

void Input::hideCursor() { m_windowImpl.hideCursor(); }

void Input::showCursor() { m_windowImpl.showCursor(); }

void Input::update() { calculateMousePositionDelta(); }

void Input::calculateMousePositionDelta() {
    auto mousePosition      = m_windowImpl.getMousePosition();
    m_mousePositionDelta    = mousePosition - m_previousMousePosition;
    m_previousMousePosition = mousePosition;
}

bool Input::isKeyPressed(WindowImpl::Key keyCode) const {
    return m_windowImpl.isKeyPressed(keyCode);
}

bool Input::isMouseButtonPressed(WindowImpl::Button buttonCode) const {
    return m_windowImpl.isMouseButtonPressed(buttonCode);
}

}  // namespace sl

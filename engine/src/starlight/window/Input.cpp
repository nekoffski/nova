#include "Input.hh"

#include "starlight/event/EventProxy.hh"
#include "Events.hh"

namespace sl {

Input::Input(Window::Impl& window
) : m_window(window), m_previousMousePosition(m_window.getMousePosition()) {
    setCallbacks();
}

void Input::setCallbacks() {
    m_window.onKeyCallback([](KeyAction action, int key) {
        KeyEvent event{ action, key };
        log::trace("Detected key action, emitting event: {}", event);
        EventProxy::get().emit<KeyEvent>(event);
    });

    m_window.onScrollCallback([](float offset) {
        ScrollEvent event{ offset };
        log::trace("Detected scroll, emitting event: {}", event);
        EventProxy::get().emit<ScrollEvent>(event);
    });

    m_window.onMouseCallback([](MouseAction action, int button) {
        MouseEvent event{ action, button };
        log::trace("Detected mouse action, emitting event: {}", event);
        EventProxy::get().emit<MouseEvent>(event);
    });
}

Vec2<f32> Input::getMousePosition() const { return m_window.getMousePosition(); }

Vec2<f32> Input::getMousePositionDelta() const { return m_mousePositionDelta; }

void Input::hideCursor() { m_window.hideCursor(); }

void Input::showCursor() { m_window.showCursor(); }

void Input::update() { calculateMousePositionDelta(); }

void Input::calculateMousePositionDelta() {
    auto mousePosition      = m_window.getMousePosition();
    m_mousePositionDelta    = mousePosition - m_previousMousePosition;
    m_previousMousePosition = mousePosition;
}

bool Input::isKeyPressed(Window::Key keyCode) const {
    return m_window.isKeyPressed(keyCode);
}

bool Input::isMouseButtonPressed(Window::Button buttonCode) const {
    return m_window.isMouseButtonPressed(buttonCode);
}

}  // namespace sl

#include "Window.hh"

#include <kc/core/Log.h>

#include "starlight/core/event/EventProxy.hh"
#include "starlight/core/event/Quit.hh"
#include "starlight/core/event/WindowResized.hh"

namespace sl {

Window::Window(WindowImpl& window) : m_windowImpl(window) { setCallbacks(); }

void Window::setCallbacks() {
    m_windowImpl.onWindowCloseCallback([]() {
        LOG_INFO("Window closed, emitting event");
        EventProxy::get().emit<QuitEvent>("Window closed");
    });

    m_windowImpl.onWindowResizeCallback([](uint32_t width, uint32_t height) {
        WindowResized event{
            Vec2<u32>{ width, height }
        };
        LOG_TRACE("Window resized, emitting event: {}", event);
        EventProxy::get().emit<WindowResized>(event);
    });
}

Vec2<u32> Window::getSize() const { return m_windowImpl.getSize(); }

Vec2<u32> Window::getFramebufferSize() const {
    return m_windowImpl.getFramebufferSize();
}

void* Window::getHandle() { return m_windowImpl.getHandle(); }

u32 Window::getFramebufferWidth() const { return getFramebufferSize().x; }
u32 Window::getFramebufferHeight() const { return getFramebufferSize().y; }

u32 Window::getWidth() const { return getSize().x; }
u32 Window::getHeight() const { return getSize().y; }

}  // namespace sl

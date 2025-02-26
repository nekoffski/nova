#include "Window.hh"

#include "Events.hh"
#include "starlight/event/EventProxy.hh"
#include "starlight/core/Globals.hh"

#ifdef SL_USE_GLFW
#include "glfw/GLFWWindow.hh"
#endif

namespace sl {

Window::Window() : m_impl(Impl::create()) { setCallbacks(); }

void Window::setCallbacks() {
    m_impl->onWindowCloseCallback([]() {
        log::info("Window closed, emitting event");
        EventProxy::get().emit<QuitEvent>("Window closed");
    });

    m_impl->onWindowResizeCallback([](uint32_t width, uint32_t height) {
        WindowResized event{
            Vec2<u32>{ width, height }
        };
        log::trace("Window resized, emitting event: {}", event);
        EventProxy::get().emit<WindowResized>(event);
    });
}

Vec2<u32> Window::getSize() const { return m_impl->getSize(); }

Vec2<u32> Window::getFramebufferSize() const { return m_impl->getFramebufferSize(); }

void* Window::getHandle() { return m_impl->getHandle(); }

Window::Impl& Window::getImpl() { return *m_impl; }

u32 Window::getFramebufferWidth() const { return getFramebufferSize().x; }
u32 Window::getFramebufferHeight() const { return getFramebufferSize().y; }

u32 Window::getWidth() const { return getSize().x; }
u32 Window::getHeight() const { return getSize().y; }

UniquePtr<Window::Impl> Window::Impl::create() {
    const auto& windowConfig = Globals::get().getConfig().window;
#ifdef SL_USE_GLFW
    return UniquePtr<glfw::GLFWWindow>::create(windowConfig);
#else
    log::panic("Window provider not specified");
#endif
}

}  // namespace sl

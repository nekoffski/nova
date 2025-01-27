#include "Context.hh"

#include "Log.hh"

namespace sl {

Context::Context(const Config& config) :
    m_config(config), m_windowImpl(config.window), m_window(m_windowImpl),
    m_input(m_windowImpl) {}

float Context::beginFrame() {
    const auto deltaTime = m_clock.getDeltaTime();

    m_windowImpl.update();
    m_eventBroker.dispatch();

    return deltaTime;
}

void Context::endFrame() {
    m_windowImpl.swapBuffers();

    m_clock.update();
    m_input.update();
}

Window& Context::getWindow() { return m_window; }

EventProxy& Context::getEventProxy() { return m_eventBroker.getProxy(); }

const Config& Context::getConfig() const { return m_config; }

}  // namespace sl

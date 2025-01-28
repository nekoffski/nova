#include "Context.hh"

#include "Log.hh"

namespace sl {

Context::Context(const Config& config) :
    m_config(config), m_windowImpl(config.window), m_window(m_windowImpl),
    m_input(m_windowImpl) {}

float Context::beginFrame() {
    m_taskQueue.dispatchQueue(TaskQueue::Type::preFrame);

    m_windowImpl.update();
    m_eventBroker.dispatch();

    return m_clock.getDeltaTime();
}

void Context::endFrame() {
    m_windowImpl.swapBuffers();

    m_clock.update();
    m_input.update();

    m_taskQueue.dispatchQueue(TaskQueue::Type::postFrame);
}

Window& Context::getWindow() { return m_window; }

EventProxy& Context::getEventProxy() { return m_eventBroker.getProxy(); }

const Config& Context::getConfig() const { return m_config; }

}  // namespace sl

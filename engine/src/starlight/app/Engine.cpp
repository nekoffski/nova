#include "Engine.hh"

namespace sl {

Engine::Engine(const Config& config) :
    m_globals(config), m_isRunning(true), m_eventProxy(m_eventBroker.getProxy()),
    m_eventSentinel(m_eventProxy), m_input(m_window.getImpl()) {
    initEvents();
}

void Engine::run() {
    while (m_isRunning) {
        const auto frameTime = beginFrame();
        updateFrame(frameTime);
        render();
        endFrame();
    }
}

void Engine::render() {}

float Engine::beginFrame() {
    m_taskQueue.dispatchQueue(TaskQueue::Type::preFrame);

    m_window.getImpl().update();
    m_eventBroker.dispatch();

    return m_clock.getDeltaTime();
}

void Engine::endFrame() {
    m_window.getImpl().swapBuffers();

    m_clock.update();
    m_input.update();

    m_taskQueue.dispatchQueue(TaskQueue::Type::postFrame);
}

void Engine::initEvents() {
    m_eventSentinel
      .add<sl::KeyEvent>([&](auto& event) {
          if (event.action == sl::KeyAction::press && event.key == SL_KEY_ESCAPE) {
              log::debug("ESC pressed, quit requested");
              m_isRunning = false;
          }
      })
      .add<QuitEvent>([&](auto& event) {
          log::debug("Got QuitEvent reason: {}, quit requested", event.reason);
          m_isRunning = false;
      });
}

}  // namespace sl

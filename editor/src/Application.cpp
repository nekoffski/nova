#include "Application.hh"

#include <starlight/core/event/Events.hh>

namespace sle {

Application::Application(const sl::Config& config) :
    m_isRunning(true), m_context(config), m_eventSentinel(m_context.getEventProxy()),
    m_renderer(m_context.getWindow(), config) {}

int Application::start() {
    init();
    startRenderLoop();

    return 0;
}

void Application::init() { initEvents(); }

void Application::onFrame(float deltaTime) {}

void Application::startRenderLoop() {
    while (m_isRunning)
        m_context.beginFrame([&](float deltaTime) { onFrame(deltaTime); });
}

void Application::exit() { m_isRunning.store(false); }

void Application::initEvents() {
    m_eventSentinel
      .pushHandler<sl::QuitEvent>([&](const auto& event) {
          LOG_INFO("Received quit request: '{}'", event.reason);
          exit();
          return sl::EventChainBehaviour::stop;
      })
      .pushHandler<sl::KeyEvent>([&](const auto& event) {
          if (event.key == SL_KEY_ESCAPE && event.action == sl::KeyAction::press) {
              LOG_INFO("Key ESC pressed, quitting");
              exit();
          }
          return sl::EventChainBehaviour::propagate;
      });
}

}  // namespace sle

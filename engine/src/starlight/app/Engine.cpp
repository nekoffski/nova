#include "Engine.hh"

namespace sl {

Engine::Engine(const Config& config) :
    m_globals(config), m_isRunning(true), m_eventProxy(m_eventBroker.getProxy()),
    m_eventSentinel(m_eventProxy), m_input(m_window.getImpl()),
    m_defaultScene(&m_defaultCamera), m_defaultRenderGraph(m_renderer),
    m_camera(&m_defaultCamera), m_scene(&m_defaultScene),
    m_renderGraph(&m_defaultRenderGraph) {
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

void Engine::render() {
    auto renderPacket = m_scene->getRenderPacket();
    m_renderGraph->render(renderPacket);
}

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

Scene* Engine::getScene() { return m_scene; }

RenderGraph* Engine::getRenderGraph() { return m_renderGraph; }

void Engine::updateFrame(float frameTime) {
    m_camera->update(frameTime);
    update(frameTime);
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

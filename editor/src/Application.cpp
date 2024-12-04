#include "Application.hh"

#include <starlight/core/event/Events.hh>
#include <starlight/renderer/views/UIRenderView.hh>
#include <starlight/renderer/views/WorldRenderView.hh>
#include <starlight/renderer/views/SkyboxRenderView.hh>
#include <starlight/ui/fonts/FontAwesome.hh>
#include <starlight/ui/UI.hh>
#include <starlight/scene/Scene.hh>

namespace sle {

Application::Application(const sl::Config& config) :
    m_isRunning(true), m_config(config), m_context(config),
    m_window(m_context.getWindow()), m_renderer(m_context.getWindow(), config),
    m_eventSentinel(m_context.getEventProxy()),
    m_cameras(m_window.getFramebufferSize()),
    m_scene(m_window, m_cameras.getActive()),
    m_userInterface(m_window.getFramebufferSize(), &m_scene) {}

int Application::start() {
    init();
    startRenderLoop();

    return 0;
}

void Application::init() { initEvents(); }

void Application::startRenderLoop() {
    auto viewport = m_window.getFramebufferSize();

    sl::Font::SubfontProperties icons{
        "/home/nek0/kapik/projects/starlight/assets/fonts/fa-solid-900.ttf",
        ICON_MIN_FA, ICON_MAX_FA
    };
    sl::Font::Properties font{
        .name = "main-font",
        .path =
          "/home/nek0/kapik/projects/starlight/assets/fonts/Roboto-Regular.ttf",
        .size     = 15,
        .subfonts = { icons }
    };

    // auto skybox = sl::Skybox::load("skybox2/skybox");

    // m_scene.setSkybox(*skybox);

    auto worldShader = sl::Shader::load("Builtin.Shader.Material");

    auto renderGraph =
      sl::RenderGraph::Builder{ m_renderer.getRendererBackend(), viewport }
        .addView<sl::WorldRenderView>(worldShader.get())
        .addView<sl::UIRenderView>(
          std::vector<sl::Font::Properties>{ font },
          [&]() { m_userInterface.render(); }
        )
        .build();

    m_userInterface.setRenderGraph(*renderGraph);

    while (m_isRunning) {
        m_context.beginFrame([&](float deltaTime) {
            m_renderer.renderFrame(
              deltaTime, m_scene.getRenderPacket(), *renderGraph
            );
            m_cameras.update(deltaTime);
        });
    }
}

void Application::exit() { m_isRunning.store(false); }

void Application::onViewportResize(const sl::Vec2<sl::u32>& viewport) {
    m_renderer.onViewportResize(viewport);
    m_userInterface.onViewportReisze(viewport);
}

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
      })
      .pushHandler<sl::WindowResized>([&](const auto& event) {
          onViewportResize(event.size);
          return sl::EventChainBehaviour::propagate;
      });
}

}  // namespace sle

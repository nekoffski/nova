#include "Application.hh"

#include <starlight/core/event/Events.hh>
#include <starlight/renderer/passes/UIRenderPass.hh>
#include <starlight/renderer/passes/WorldRenderPass.hh>
#include <starlight/renderer/passes/SkyboxRenderPass.hh>
// #include <starlight/renderer/passes/LightsDebugRenderPass.hh>
#include <starlight/renderer/passes/ShadowMapsRenderPass.hh>
#include <starlight/renderer/passes/GridRenderPass.hh>
#include <starlight/ui/fonts/FontAwesome.hh>
#include <starlight/ui/UI.hh>
#include <starlight/scene/Scene.hh>

#include "Events.hh"

namespace sle {

Application::Application(
  const sl::Config& config, std::optional<std::string> scenePath
) :
    m_isRunning(true), m_config(config), m_context(config),
    m_eventProxy(m_context.getEventProxy()), m_window(m_context.getWindow()),
    m_renderer(m_context), m_eventSentinel(m_eventProxy),
    m_cameras(m_window.getFramebufferSize(), m_eventProxy),
    m_scene(m_window, m_cameras.getActive()),
    m_userInterface(m_eventProxy, m_window.getFramebufferSize(), &m_scene) {
    if (scenePath) {
        LOG_INFO("Loading initial scene: {}", *scenePath);
        m_sceneParser.deserialize(m_scene, *scenePath);
    }
}

int Application::start() {
    init();
    startRenderLoop();

    return 0;
}

void Application::init() { initEvents(); }

void Application::startRenderLoop() {
    auto viewport = m_window.getFramebufferSize();

    sl::Vec2<sl::f32> viewportOffset{
        m_userInterface.getConfig().panelWidthRatio,
        m_userInterface.getConfig().panelHeightRatio
    };

    auto worldShader = sl::ShaderFactory::get().load("Builtin.Shader.Material");

    sl::RenderGraph renderGraph{ m_renderer };

    renderGraph.addRenderPass<sl::SkyboxRenderPass>();
    renderGraph.addRenderPass<sl::ShadowMapsRenderPass>();
    renderGraph.addRenderPass<sl::WorldRenderPass>();
    renderGraph.addRenderPass<sl::GridRenderPass>();
    renderGraph.addRenderPass<sl::UIRenderPass>(m_userInterface);

    m_userInterface.setRenderGraph(renderGraph);

    while (m_isRunning) {
        auto renderPacket = m_scene.getRenderPacket();
        m_context.beginFrame([&](float deltaTime) {
            renderGraph.render(renderPacket);
            m_cameras.update(deltaTime);
        });
    }
}

void Application::exit() { m_isRunning.store(false); }

void Application::initEvents() {
    m_eventSentinel
      .add<sl::QuitEvent>([&](const auto& event, auto&& handled) {
          LOG_INFO("Received quit request: '{}'", event.reason);
          exit();
          handled();
      })
      .add<sl::KeyEvent>([&](const auto& event) {
          if (event.key == SL_KEY_ESCAPE && event.action == sl::KeyAction::press) {
              LOG_INFO("Key ESC pressed, quitting");
              exit();
          } else if (event.key == SL_KEY_Z) {
              //   m_renderer.setRenderMode(sl::RenderMode::lights);
          } else if (event.key == SL_KEY_X) {
              //   m_renderer.setRenderMode(sl::RenderMode::normals);
          } else if (event.key == SL_KEY_C) {
              //   m_renderer.setRenderMode(sl::RenderMode::standard);
          }
      })
      .add<events::SceneSerialization>([&](const auto& event, auto&& handled) {
          if (event.action == events::SceneSerialization::Action::serialize) {
              EDITOR_LOG_DEBUG("Serializing scene: {}", event.path);
              m_sceneParser.serialize(m_scene, event.path);
          } else {
              EDITOR_LOG_DEBUG("Deserializing scene: {}", event.path);
              m_scene.clear();
              m_sceneParser.deserialize(m_scene, event.path);
          }
          handled();
      });
}

}  // namespace sle

#include "Application.hh"

#include <starlight/core/event/Events.hh>
#include <starlight/renderer/views/UIRenderView.hh>
#include <starlight/renderer/views/WorldRenderView.hh>
#include <starlight/renderer/views/SkyboxRenderView.hh>
#include <starlight/renderer/views/LightsDebugRenderView.hh>
#include <starlight/renderer/views/ShadowMapsRenderView.hh>
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
    m_renderer(m_context.getWindow(), m_eventProxy, config),
    m_eventSentinel(m_eventProxy),
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

    sl::Vec2<sl::f32> viewportOffset{
        m_userInterface.getConfig().panelWidthRatio,
        m_userInterface.getConfig().panelHeightRatio
    };

    auto worldShader = sl::ShaderFactory::get().load("Builtin.Shader.Material");

    auto& backend = m_renderer.getRendererBackend();
    auto renderGraph =
      sl::RenderGraph::Builder{ backend, m_eventProxy, viewport }
        .addView<sl::SkyboxRenderView>(viewportOffset)
        .addView<sl::ShadowMapsRenderView>(viewportOffset)
        .addView<sl::WorldRenderView>(viewportOffset, worldShader)
        .addView<sl::LightsDebugRenderView>(viewportOffset)
        .addView<sl::UIRenderView>(
          std::vector<sl::Font::Properties>{ font },
          [&]() { m_userInterface.render(); }
        )
        .build();

    m_userInterface.setRenderGraph(*renderGraph);

    while (m_isRunning) {
        auto renderPacket = m_scene.getRenderPacket();
        m_context.beginFrame([&](float deltaTime) {
            m_renderer.renderFrame(deltaTime, renderPacket, *renderGraph);
            m_cameras.update(deltaTime);
        });
    }
}

void Application::exit() { m_isRunning.store(false); }

void Application::initEvents() {
    m_eventSentinel
      .add<sl::QuitEvent>([&](const auto& event) {
          LOG_INFO("Received quit request: '{}'", event.reason);
          exit();
          return sl::EventChainBehaviour::stop;
      })
      .add<sl::KeyEvent>([&](const auto& event) {
          if (event.key == SL_KEY_ESCAPE && event.action == sl::KeyAction::press) {
              LOG_INFO("Key ESC pressed, quitting");
              exit();
          } else if (event.key == SL_KEY_Z) {
              m_renderer.setRenderMode(sl::RenderMode::lights);
          } else if (event.key == SL_KEY_X) {
              m_renderer.setRenderMode(sl::RenderMode::normals);
          } else if (event.key == SL_KEY_C) {
              m_renderer.setRenderMode(sl::RenderMode::standard);
          }
          return sl::EventChainBehaviour::propagate;
      })
      .add<events::SceneSerialization>([&](const auto& event) {
          if (event.action == events::SceneSerialization::Action::serialize) {
              EDITOR_LOG_DEBUG("Serializing scene: {}", event.path);
              m_sceneParser.serialize(m_scene, event.path);
          } else {
              EDITOR_LOG_DEBUG("Deserializing scene: {}", event.path);
              m_scene.clear();
              m_sceneParser.deserialize(m_scene, event.path);
          }
          return sl::EventChainBehaviour::stop;
      });
}

}  // namespace sle

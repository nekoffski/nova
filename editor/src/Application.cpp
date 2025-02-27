#include "Application.hh"

#include <starlight/window/Events.hh>
#include <starlight/app/renderPasses/UIRenderPass.hh>
#include <starlight/app/renderPasses/WorldRenderPass.hh>
#include <starlight/app/renderPasses/SkyboxRenderPass.hh>
// #include <starlight/app/renderPasses/LightsDebugRenderPass.hh>
#include <starlight/app/renderPasses/ShadowMapsRenderPass.hh>
#include <starlight/app/renderPasses/GridRenderPass.hh>
#include <starlight/ui/fonts/FontAwesome.hh>
#include <starlight/ui/UI.hh>
#include <starlight/app/scene/Scene.hh>

#include "Events.hh"

namespace sle {

Application::Application(
  const sl::Config& config, std::optional<std::string> scenePath
) :
    Engine(config), m_eventSentinel(sl::EventProxy::get()),
    m_cameras(sl::Window::get().getFramebufferSize()),
    m_userInterface(sl::Window::get().getFramebufferSize(), getScene()) {
    if (scenePath) {
        sl::log::info("Loading initial scene: {}", *scenePath);
        m_sceneParser.deserialize(*getScene(), *scenePath);
    }

    initEvents();

    sl::Vec2<sl::f32> viewportOffset{
        m_userInterface.getConfig().panelWidthRatio,
        m_userInterface.getConfig().panelHeightRatio
    };

    getRenderGraph()->addPass<sl::SkyboxRenderPass>(viewportOffset);
    getRenderGraph()->addPass<sl::ShadowMapsRenderPass>();
    getRenderGraph()->addPass<sl::WorldRenderPass>(viewportOffset);
    getRenderGraph()->addPass<sl::GridRenderPass>(viewportOffset);
    getRenderGraph()->addPass<sl::UIRenderPass>(m_userInterface);

    m_userInterface.setRenderGraph(*getRenderGraph());
}

void Application::update([[maybe_unused]] float frameTime) {}

void Application::initEvents() {
    m_eventSentinel
      .add<sl::KeyEvent>([&](const auto& event) {
          if (event.key == SL_KEY_Z) {
              //   m_renderer.setRenderMode(sl::RenderMode::lights);
          } else if (event.key == SL_KEY_X) {
              //   m_renderer.setRenderMode(sl::RenderMode::normals);
          } else if (event.key == SL_KEY_C) {
              //   m_renderer.setRenderMode(sl::RenderMode::standard);
          }
      })
      .add<events::SceneSerialization>([&](const auto& event, auto&& handled) {
          if (event.action == events::SceneSerialization::Action::serialize) {
              editorWriteDebug("Serializing scene: {}", event.path);
              m_sceneParser.serialize(*getScene(), event.path);
          } else {
              editorWriteDebug("Deserializing scene: {}", event.path);
              getScene()->clear();
              m_sceneParser.deserialize(*getScene(), event.path);
          }
          handled();
      });
}

}  // namespace sle

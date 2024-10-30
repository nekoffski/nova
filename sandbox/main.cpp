

#include <csignal>

#include "starlight/core/Core.hh"
#include "starlight/core/window/Window.hh"
#include "starlight/core/Context.hh"
#include "starlight/core/event/Quit.hh"

#include "starlight/renderer/views/WorldRenderView.hh"
#include "starlight/renderer/camera/EulerCamera.hh"
#include "starlight/renderer/views/SkyboxRenderView.hh"
#include "starlight/renderer/RendererFrontend.hh"
#include "starlight/renderer/RenderGraph.hh"
#include "starlight/renderer/gpu/Shader.hh"
#include "starlight/renderer/light/PointLight.hh"
#include "starlight/scene/Scene.hh"
#include "starlight/renderer/MeshComposite.hh"

static std::atomic_bool isRunning = true;

int main() {
    std::signal(SIGINT, []([[maybe_unused]] int) { isRunning = false; });

    sl::Context context;
    sl::RendererFrontend renderer{ context };

    auto& window            = context.getWindow();
    const auto viewportSize = window.getFramebufferSize();

    auto& eventProxy = sl::EventProxy::get();
    sl::EventHandlerSentinel sentinel{ eventProxy };

    sentinel
      .pushHandler<sl::QuitEvent>([&]([[maybe_unused]] const auto& ev) {
          isRunning = false;
          return sl::EventChainBehaviour::propagate;
      })
      .pushHandler<sl::KeyEvent>([&]([[maybe_unused]] const auto& ev) {
          if (ev.key == SL_KEY_ESCAPE) isRunning = false;
          return sl::EventChainBehaviour::propagate;
      });

    sl::EulerCamera camera(sl::EulerCamera::Properties{
      .target       = sl::Vec3<sl::f32>{ 0.0f },
      .radius       = 5.0f,
      .viewportSize = viewportSize,
    });

    auto& rendererBackend = renderer.getRendererBackend();
    auto skybox           = sl::Skybox::load("skybox2/skybox");
    auto worldShader      = sl::Shader::load("Builtin.Shader.Material");

    auto renderGraph =
      sl::RenderGraph::Builder{ rendererBackend, viewportSize }
        .addView<sl::SkyboxRenderView>(skybox.get())
        .addView<sl::WorldRenderView>(worldShader.get())
        .build();

    sl::Scene scene{ window, &camera };

    auto& entity = scene.addEntity();

    entity
      .addComponent<sl::MeshComposite>(
        sl::Mesh::getCube(), sl::Material::load("Builtin.Material.Test")
      )
      .data()
      .getRoot()
      .getInstances()
      .front()
      .scale(sl::Vec3<sl::f32>{ 0.25f });

    while (isRunning) {
        context.beginFrame([&](float deltaTime) {
            renderer.renderFrame(deltaTime, scene.getRenderPacket(), *renderGraph);
            camera.update(deltaTime);
        });
    }

    return 0;
}

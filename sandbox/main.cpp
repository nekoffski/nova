

#include <csignal>

#include "starlight/core/Core.hh"
#include "starlight/core/window/Window.hh"
#include "starlight/core/Context.hh"
#include "starlight/core/event/Events.hh"

#include "starlight/renderer/views/WorldRenderView.hh"
#include "starlight/renderer/views/LightsDebugRenderView.hh"
#include "starlight/renderer/views/SkyboxRenderView.hh"
#include "starlight/renderer/views/UIRenderView.hh"
#include "starlight/renderer/camera/EulerCamera.hh"
#include "starlight/renderer/RendererFrontend.hh"
#include "starlight/renderer/RenderGraph.hh"
#include "starlight/renderer/gpu/Shader.hh"
#include "starlight/renderer/light/PointLight.hh"
#include "starlight/scene/Scene.hh"
#include "starlight/renderer/MeshComposite.hh"
#include "starlight/ui/UI.hh"
#include "starlight/ui/fonts/FontAwesome.hh"

static std::atomic_bool isRunning = true;

int main(int argc, char** argv) {
    sl::initLogging("sl-sandbox");

    ASSERT(argc >= 2, "Config path required");
    auto config = sl::Config::fromJson(std::string{ argv[1] });
    ASSERT(config, "Could not load config file");

    std::signal(SIGINT, []([[maybe_unused]] int) { isRunning = false; });

    sl::Context context{ *config };
    auto& window     = context.getWindow();
    auto& eventProxy = sl::EventProxy::get();

    sl::RendererFrontend renderer{ window, eventProxy, *config };

    const auto viewportSize = window.getFramebufferSize();

    sl::EulerCamera camera(
      sl::EulerCamera::Properties{
        .target       = sl::Vec3<sl::f32>{ 0.0f },
        .radius       = 5.0f,
        .viewportSize = viewportSize,
      },
      eventProxy
    );

    auto& rendererBackend = renderer.getRendererBackend();
    auto skybox           = sl::Skybox::load("skybox2/skybox");
    auto worldShader      = sl::Shader::load("Builtin.Shader.Material");

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

    const auto viewportOffset = sl::Vec2<sl::f32>{ 0.0f, 0.0f };

    auto renderGraph =
      sl::RenderGraph::Builder{ rendererBackend, eventProxy, viewportSize }
        .addView<sl::SkyboxRenderView>(viewportOffset)
        .addView<sl::WorldRenderView>(viewportOffset, worldShader)
        .addView<sl::LightsDebugRenderView>(viewportOffset)
        .addView<sl::UIRenderView>(
          std::vector<sl::Font::Properties>{ font },
          []() { sl::ui::text("Hello world!"); }
        )
        .build();

    sl::EventHandlerSentinel sentinel{ eventProxy };

    sentinel
      .add<sl::QuitEvent>([&]([[maybe_unused]] const auto& ev) {
          isRunning = false;
          return sl::EventChainBehaviour::propagate;
      })
      .add<sl::KeyEvent>([&](const auto& ev) {
          if (ev.key == SL_KEY_ESCAPE)
              isRunning = false;
          else if (ev.key == SL_KEY_Z)
              renderer.setRenderMode(sl::RenderMode::lights);
          else if (ev.key == SL_KEY_X)
              renderer.setRenderMode(sl::RenderMode::normals);
          else if (ev.key == SL_KEY_C)
              renderer.setRenderMode(sl::RenderMode::standard);
          return sl::EventChainBehaviour::propagate;
      });

    sl::Scene scene{ window, &camera };
    scene.setSkybox(*skybox);

    auto& entity = scene.addEntity();

    auto& instance =
      entity
        .addComponent<sl::MeshComposite>(
          sl::Mesh::getCube(), sl::Material::load("Builtin.Material.Test")
        )
        .data()
        .getRoot()
        .getInstances()
        .front();

    while (isRunning) {
        context.beginFrame([&](float deltaTime) {
            renderer.renderFrame(deltaTime, scene.getRenderPacket(), *renderGraph);
            camera.update(deltaTime);

            instance.rotate(
              sl::Vec3<sl::f32>{ 0.0f, 1.0f, 0.0f }, 0.25f * deltaTime
            );
        });
    }

    return 0;
}

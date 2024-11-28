

#include <csignal>

#include "starlight/core/Core.hh"
#include "starlight/core/window/Window.hh"
#include "starlight/core/Context.hh"
#include "starlight/core/event/Quit.hh"

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
    ASSERT(argc >= 2, "Config path required");
    auto config = sl::Config::fromJson(std::string{ argv[1] });
    ASSERT(config, "Could not load config file");

    std::signal(SIGINT, []([[maybe_unused]] int) { isRunning = false; });

    sl::Context context{ *config };
    auto& window = context.getWindow();

    sl::RendererFrontend renderer{ window, *config };

    const auto viewportSize = window.getFramebufferSize();

    auto& eventProxy = sl::EventProxy::get();
    sl::EventHandlerSentinel sentinel{ eventProxy };

    sentinel
      .pushHandler<sl::QuitEvent>([&]([[maybe_unused]] const auto& ev) {
          isRunning = false;
          return sl::EventChainBehaviour::propagate;
      })
      .pushHandler<sl::KeyEvent>([&]([[maybe_unused]] const auto& ev) {
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

    sl::EulerCamera camera(sl::EulerCamera::Properties{
      .target       = sl::Vec3<sl::f32>{ 0.0f },
      .radius       = 5.0f,
      .viewportSize = viewportSize,
    });

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

    auto renderGraph =
      sl::RenderGraph::Builder{ rendererBackend, viewportSize }
        .addView<sl::SkyboxRenderView>(skybox.get())
        .addView<sl::WorldRenderView>(worldShader.get())
        .addView<sl::UIRenderView>(
          std::vector<sl::Font::Properties>{ font },
          []() { sl::ui::text("Hello world!"); }
        )
        .build();

    sl::Scene scene{ window, &camera };

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

    instance.scale(sl::Vec3<sl::f32>{ 0.1f });

    renderer.setRenderGraph(renderGraph.get());

    while (isRunning) {
        context.beginFrame([&](float deltaTime) {
            renderer.renderFrame(deltaTime, scene.getRenderPacket());
            camera.update(deltaTime);

            instance.rotate(
              sl::Vec3<sl::f32>{ 0.0f, 1.0f, 0.0f }, 0.25f * deltaTime
            );
        });
    }

    return 0;
}

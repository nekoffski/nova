

#include <csignal>

#include "starlight/core/Core.hh"
#include "starlight/core/window/Window.hh"
#include "starlight/core/Context.hh"
#include "starlight/core/event/Events.hh"

#include "starlight/renderer/views/WorldRenderView.hh"
#include "starlight/renderer/views/LightsDebugRenderView.hh"
#include "starlight/renderer/views/SkyboxRenderView.hh"
#include "starlight/renderer/views/UIRenderView.hh"
#include "starlight/renderer/views/ShadowMapsRenderView.hh"
#include "starlight/renderer/camera/EulerCamera.hh"
#include "starlight/renderer/Renderer.hh"
#include "starlight/renderer/RenderGraph.hh"
#include "starlight/renderer/gpu/Shader.hh"
#include "starlight/renderer/light/PointLight.hh"
#include "starlight/scene/Scene.hh"
#include "starlight/renderer/MeshComposite.hh"
#include "starlight/ui/UI.hh"
#include "starlight/ui/fonts/FontAwesome.hh"
#include "starlight/renderer/passes/DummyRenderPass.hh"

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

    sl::Renderer renderer{ context };
    sl::RenderGraph renderGraph{ renderer };

    renderGraph.addRenderPass<sl::DummyRenderPass>();

    const auto viewportSize = window.getFramebufferSize();

    sl::EulerCamera camera(
      sl::EulerCamera::Properties{
        .target       = sl::Vec3<sl::f32>{ 0.0f },
        .radius       = 5.0f,
        .viewportSize = viewportSize,
      },
      eventProxy
    );

    sl::Scene scene{ window, &camera };

    int frames = 2;

    while (isRunning) {
        context.beginFrame([&](float deltaTime) {
            auto renderPacket = scene.getRenderPacket();
            renderGraph.render(renderPacket);
        });

        if (--frames <= 0) break;
    }

    return 0;
}

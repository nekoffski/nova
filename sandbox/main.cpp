

#include <csignal>

#include "starlight/core/Core.hh"
#include "starlight/core/window/Window.hh"
#include "starlight/core/Context.hh"
#include "starlight/core/event/Events.hh"

#include "starlight/renderer/camera/EulerCamera.hh"
#include "starlight/renderer/Renderer.hh"
#include "starlight/renderer/RenderGraph.hh"
#include "starlight/renderer/gpu/Shader.hh"
#include "starlight/renderer/light/PointLight.hh"
#include "starlight/scene/Scene.hh"
#include "starlight/renderer/MeshComposite.hh"
#include "starlight/ui/fonts/FontAwesome.hh"
#include "starlight/renderer/passes/SkyboxRenderPass.hh"
#include "starlight/renderer/passes/GridRenderPass.hh"
#include "starlight/renderer/passes/WorldRenderPass.hh"
#include "starlight/renderer/passes/ShadowMapsRenderPass.hh"
#include "starlight/renderer/passes/UIRenderPass.hh"
#include "starlight/ui/UI.hh"

#include "starlight/core/Utils.hh"

#include <iostream>

struct UI : sl::UI {
    void render() { ImGui::Text("hello world"); }
};

static std::atomic_bool isRunning = true;

int main(int argc, char** argv) {
    sl::log::init("sl-sandbox");

    sl::log::expect(argc >= 2, "Config path required");
    auto config = sl::Config::fromJson(std::string{ argv[1] });
    sl::log::expect(config.has_value(), "Could not load config file");

    std::signal(SIGINT, []([[maybe_unused]] int) { isRunning = false; });

    sl::Context context{ *config };
    // auto& window     = context.getWindow();
    auto& eventProxy = sl::EventProxy::get();

    eventProxy.pushEventHandler<sl::KeyEvent>([&](auto& event) {
        if (event.action == sl::KeyAction::press && event.key == SL_KEY_ESCAPE)
            isRunning = false;
    });

    sl::Renderer renderer{ context };

    const auto shaderName = "Builtin.Shader.Material";
    auto shaderProgram    = sl::ShaderProgramFactory::get().load(shaderName);

    // sl::RenderGraph renderGraph{ renderer };

    // UI ui{};

    // sl::Vec2<sl::f32> viewportOffset{ 0.0f, 0.0f };

    // renderGraph.addRenderPass<sl::SkyboxRenderPass>(viewportOffset);
    // renderGraph.addRenderPass<sl::ShadowMapsRenderPass>();
    // renderGraph.addRenderPass<sl::WorldRenderPass>(viewportOffset);
    // renderGraph.addRenderPass<sl::GridRenderPass>(viewportOffset);
    // renderGraph.addRenderPass<sl::UIRenderPass>(ui);

    // const auto viewportSize = window.getFramebufferSize();

    // sl::EulerCamera camera(
    //   sl::EulerCamera::Properties{
    //     .target       = sl::Vec3<sl::f32>{ 0.0f },
    //     .radius       = 5.0f,
    //     .viewportSize = viewportSize,
    //   },
    //   eventProxy
    // );

    // sl::Scene scene{ window, &camera };
    // scene.skybox = sl::SkyboxFactory::get().load("skybox2/skybox");

    // auto& entity = scene.addEntity();
    // entity.addComponent<sl::MeshComposite>(
    //   sl::MeshFactory::get().getCube(),
    //   sl::MaterialFactory::get().load("Builtin.Material.Test")
    // );

    // while (isRunning) {
    //     context.beginFrame([&](float deltaTime) {
    //         auto renderPacket = scene.getRenderPacket();
    //         renderGraph.render(renderPacket);

    //         camera.update(deltaTime);
    //     });
    // }

    sl::log::info("Sandbox exited");
    return 0;
}

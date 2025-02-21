

#include <csignal>

#include "starlight/app/Engine.hh"

// struct UI : sl::UI {
//     void render() { ImGui::Text("hello world"); }
// };

class Sandbox : public sl::Engine {
public:
    explicit Sandbox(const sl::Config& config) : Engine(config) {}

private:
    void updateFrame(float frameTime) override {}
};

int main(int argc, char** argv) {
    sl::log::init("sl-sandbox");
    sl::log::expect(argc >= 2, "Config path required");
    auto config = sl::Config::fromJson(std::string{ argv[1] });
    sl::log::expect(config.has_value(), "Could not load config file");

    Sandbox sandbox{ *config };
    sandbox.run();

    // sl::Context context{ *config };
    // auto& window     = context.getWindow();
    // auto& eventProxy = sl::EventProxy::get();

    // sl::Renderer renderer{ context };
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

    return 0;
}

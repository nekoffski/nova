

#include <csignal>

#include "starlight/app/Engine.hh"
#include "starlight/ui/UI.hh"
#include "starlight/renderer/passes/GridRenderPass.hh"
#include "starlight/renderer/passes/SkyboxRenderPass.hh"
#include "starlight/renderer/passes/ShadowMapsRenderPass.hh"
#include "starlight/renderer/passes/WorldRenderPass.hh"
#include "starlight/renderer/passes/UIRenderPass.hh"
#include "starlight/renderer/MeshComposite.hh"

struct UI : sl::UI {
    void render() { ImGui::Text("hello world"); }
};

class Sandbox : public sl::Engine {
public:
    explicit Sandbox(const sl::Config& config) : Engine(config) {
        sl::Vec2<sl::f32> viewportOffset{ 0.0f, 0.0f };

        getRenderGraph()->addPass<sl::SkyboxRenderPass>(viewportOffset);
        getRenderGraph()->addPass<sl::GridRenderPass>(viewportOffset);
        getRenderGraph()->addPass<sl::ShadowMapsRenderPass>();
        getRenderGraph()->addPass<sl::WorldRenderPass>(viewportOffset);
        getRenderGraph()->addPass<sl::UIRenderPass>(m_ui);

        getScene()->skybox = sl::SkyboxFactory::get().load("skybox2/skybox");

        auto& entity = getScene()->addEntity();
        entity.addComponent<sl::MeshComposite>(
          sl::MeshFactory::get().getCube(),
          sl::MaterialFactory::get().load("Builtin.Material.Test")
        );
    }

private:
    void update(float frameTime) override {}

    UI m_ui;
};

int main(int argc, char** argv) {
    sl::log::init("sl-sandbox");
    sl::log::expect(argc >= 2, "Config path required");
    auto config = sl::Config::fromJson(std::string{ argv[1] });
    sl::log::expect(config.has_value(), "Could not load config file");

    Sandbox sandbox{ *config };
    sandbox.run();

    return 0;
}

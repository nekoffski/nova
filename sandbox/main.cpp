

#include <csignal>

#include "starlight/app/Engine.hh"
#include "starlight/ui/UI.hh"
#include "starlight/app/renderPasses/GridRenderPass.hh"
#include "starlight/app/renderPasses/SkyboxRenderPass.hh"
#include "starlight/app/renderPasses/ShadowMapsRenderPass.hh"
#include "starlight/app/renderPasses/WorldRenderPass.hh"
#include "starlight/app/renderPasses/UIRenderPass.hh"
#include "starlight/app/scene/parsing/SceneParser.hh"
#include "starlight/renderer/MeshComposite.hh"

class Sandbox : public sl::Engine {
public:
    explicit Sandbox(
      const sl::Config& config, std::optional<std::string> scenePath
    ) : Engine(config) {
        sl::Vec2<sl::f32> viewportOffset{ 0.0f, 0.0f };

        if (scenePath) sl::SceneParser{}.deserialize(*getScene(), *scenePath);

        getRenderGraph()->addPass<sl::SkyboxRenderPass>(viewportOffset);
        getRenderGraph()->addPass<sl::ShadowMapsRenderPass>();
        getRenderGraph()->addPass<sl::WorldRenderPass>(viewportOffset);
        getRenderGraph()->addPass<sl::GridRenderPass>(viewportOffset);
    }

private:
    void update([[maybe_unused]] float frameTime) override {}
};

int main(int argc, char** argv) {
    sl::log::init("sl-sandbox");
    sl::log::expect(argc >= 2, "Config path required");

    std::optional<std::string> scenePath;
    if (argc >= 3) scenePath = std::string{ argv[2] };

    auto config = sl::Config::fromJson(std::string{ argv[1] });
    sl::log::expect(config.has_value(), "Could not load config file");

    return Sandbox{ *config, scenePath }.run();
}

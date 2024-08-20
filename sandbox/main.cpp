

#include <csignal>

#include "starlight/core/Core.h"
#include "starlight/core/window/Window.h"
#include "starlight/core/Context.h"

#include "starlight/renderer/views/WorldRenderView.h"
#include "starlight/renderer/camera/EulerCamera.h"
#include "starlight/renderer/views/SkyboxRenderView.h"
#include "starlight/renderer/RendererFrontend.h"

#include "starlight/renderer/gpu/Shader.h"

static std::atomic_bool isRunning = true;

int main() {
    std::signal(SIGINT, []([[maybe_unused]] int) { isRunning = false; });

    sl::Context context;
    sl::RendererFrontend renderer{ context };

    auto& window            = context.getWindow();
    const auto viewportSize = window.getFramebufferSize();

    sl::EulerCamera camera(sl::EulerCamera::Properties{
      .target       = sl::Vec3<sl::f32>{ 0.0f },
      .radius       = 5.0f,
      .viewportSize = viewportSize,
    });

    auto& rendererBackend = renderer.getRendererBackend();

    // auto skyboxShader = sl::Shader::load(rendererBackend,
    // "Builtin.Shader.Skybox");
    auto skybox = sl::Skybox::load("skybox2/skybox");

    // sl::SkyboxRenderView skyboxView{ &camera, skyboxShader, skybox };
    // std::vector<sl::RenderView*> renderViews{ &skyboxView };

    // renderer.init(renderViews);

    // sl::RenderPacket renderPacket;

    // renderPacket.viewport = sl::Rect2<sl::u32>{
    //     sl::Vec2<sl::u32>{0u, 0u},
    //     viewportSize
    // };

    // while (isRunning) {
    //     context.beginFrame([&](float deltaTime) {
    //         renderer.renderFrame(deltaTime, renderPacket);
    //         camera.update(deltaTime);
    //     });
    // }

    return 0;
}
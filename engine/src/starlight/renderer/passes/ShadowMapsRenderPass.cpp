#include "ShadowMapsRenderPass.hh"

#include "starlight/core/window/Window.hh"
#include "starlight/renderer/factories/TextureFactory.hh"
#include "starlight/renderer/factories/ShaderFactory.hh"
#include "starlight/renderer/Renderer.hh"

namespace sl {

static constexpr u64 shadowMapResolution = 1024;

ShadowMapsRenderPass::ShadowMapsRenderPass(Renderer& renderer
) : RenderPass(renderer, ShaderFactory::get().load("Builtin.Shader.ShadowMaps")) {}

RenderPassBackend::Properties ShadowMapsRenderPass::createProperties(
  [[maybe_unused]] bool hasPreviousPass, [[maybe_unused]] bool hasNextPass
) {
    auto& swapchain      = m_renderer.getSwapchain();
    auto depthProperties = swapchain.getDepthBuffer()->getImageData();

    depthProperties.width  = shadowMapResolution;
    depthProperties.height = shadowMapResolution;
    depthProperties.usage |= Texture::Usage::sampled;

    const auto swapchainImageCount = swapchain.getImageCount();

    auto& device = m_renderer.getDevice();
    for (u32 i = 0; i < swapchainImageCount; ++i)
        m_shadowMaps.push_back(device.createTexture(depthProperties));

    RenderPassBackend::Properties props;

    props.clearFlags = ClearFlags::depth;
    props.rect       = Rect2<u32>{
        Vec2<u32>{ 0u,                  0u                  },
        Vec2<u32>{ shadowMapResolution, shadowMapResolution }
    };

    props.renderTargets.reserve(swapchainImageCount);

    RenderTarget renderTarget;
    for (u8 i = 0; i < swapchainImageCount; ++i) {
        renderTarget.depthAttachment = m_shadowMaps[i].get();
        props.renderTargets.push_back(renderTarget);
    }

    return props;
}

void ShadowMapsRenderPass::render(
  RenderPacket& packet, CommandBuffer& commandBuffer, u32 imageIndex,
  [[maybe_unused]] u64 frameNumber
) {
    if (packet.directionalLights.empty()) return;

    auto depthMVP =
      math::ortho<float>(-5.0f, 5.0f, -5.0f, 5.0f, -5.0f, 20.0f)
      * math::lookAt(
        -packet.directionalLights[0].direction, Vec3<f32>(0.0f, 0.0f, 0.0f),
        Vec3<f32>(0.0f, 1.0f, 0.0f)
      );

    setGlobalUniforms(commandBuffer, imageIndex, [&](auto& setter) {
        setter.set("depthMVP", depthMVP);
    });

    for (auto& [worldTransform, mesh, _] : packet.entities) {
        setPushConstant(commandBuffer, "model", worldTransform);
        drawMesh(*mesh, commandBuffer);
    }
    packet.shadowMaps.push_back(m_shadowMaps[imageIndex].get());
}

Rect2<u32> ShadowMapsRenderPass::getViewport() {
    return Rect2<u32>{
        Vec2<u32>{ 0u,                  0u                  },
        Vec2<u32>{ shadowMapResolution, shadowMapResolution }
    };
}

}  // namespace sl

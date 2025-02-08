#include "SkyboxRenderPass.hh"

#include "starlight/renderer/factories/SkyboxFactory.hh"
#include "starlight/renderer/factories/MeshFactory.hh"

namespace sl {

SkyboxRenderPass::SkyboxRenderPass(
  Renderer& renderer, const Vec2<f32>& viewportOffset
) :
    RenderPass(
      renderer, SkyboxFactory::get().getDefaultShader(), viewportOffset,
      "SkyboxRenderPass"
    ) {}

RenderPassBackend::Properties SkyboxRenderPass::createProperties(
  [[maybe_unused]] bool hasPreviousPass, [[maybe_unused]] bool hasNextPass
) {
    return createDefaultProperties(Attachment::swapchainColor, ClearFlags::color);
}

void SkyboxRenderPass::render(
  RenderPacket& packet, CommandBuffer& commandBuffer, u32 imageIndex,
  [[maybe_unused]] u64 frameNumber
) {
    auto skybox = packet.skybox;

    if (not skybox) return;

    auto camera = packet.camera;
    setGlobalUniforms(commandBuffer, imageIndex, [&](auto& setter) {
        auto viewMatrix  = camera->getViewMatrix();
        viewMatrix[3][0] = 0.0f;
        viewMatrix[3][1] = 0.0f;
        viewMatrix[3][2] = 0.0f;

        setter.set("view", viewMatrix);
        setter.set("projection", camera->getProjectionMatrix());
    });
    // skybox->applyUniforms(*m_shader, commandBuffer, imageIndex);

    drawMesh(*MeshFactory::get().getCube(), commandBuffer);
}

}  // namespace sl

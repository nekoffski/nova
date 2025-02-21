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

RenderPassBackend::Properties SkyboxRenderPass::createRenderPassProperties(
  [[maybe_unused]] bool hasPreviousPass, [[maybe_unused]] bool hasNextPass
) {
    return generateRenderPassProperties(
      Attachment::swapchainColor, ClearFlags::color
    );
}

Pipeline::Properties SkyboxRenderPass::createPipelineProperties() {
    auto props     = RenderPass::createPipelineProperties();
    props.cullMode = CullMode::front;
    return props;
}

void SkyboxRenderPass::render(
  RenderPacket& packet, CommandBuffer& commandBuffer, u32 imageIndex, u64 frameNumber
) {
    auto skybox = packet.skybox;

    if (not skybox) return;

    auto camera = packet.camera;
    setGlobalUniforms(commandBuffer, frameNumber, imageIndex, [&](auto& setter) {
        auto viewMatrix  = camera->getViewMatrix();
        viewMatrix[3][0] = 0.0f;
        viewMatrix[3][1] = 0.0f;
        viewMatrix[3][2] = 0.0f;

        setter.set("view", viewMatrix);
        setter.set("projection", camera->getProjectionMatrix());
        setter.set("cubeMap", skybox->getCubeMap());
    });

    drawMesh(*MeshFactory::get().getCube(), commandBuffer);
}

}  // namespace sl

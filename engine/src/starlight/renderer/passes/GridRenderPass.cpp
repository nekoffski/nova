#include "GridRenderPass.hh"

#include "starlight/renderer/factories/ShaderFactory.hh"

namespace sl {

GridRenderPass::GridRenderPass(Renderer& renderer, const Vec2<f32>& viewportOffset) :
    RenderPass(
      renderer, ShaderFactory::get().load("Builtin.Shader.Grid"), viewportOffset,
      "GridRenderPass"
    ) {}

RenderPassBackend::Properties GridRenderPass::createRenderPassProperties(
  bool hasPreviousPass, [[maybe_unused]] bool hasNextPass
) {
    return generateRenderPassProperties(
      Attachment::swapchainColor | Attachment::depth,
      hasPreviousPass ? ClearFlags::none : ClearFlags::depth
    );
}

Pipeline::Properties GridRenderPass::createPipelineProperties() {
    auto props     = RenderPass::createPipelineProperties();
    props.cullMode = CullMode::none;
    return props;
}

void GridRenderPass::render(
  RenderPacket& packet, CommandBuffer& commandBuffer, u32 imageIndex, u64 frameNumber
) {
    setGlobalUniforms(commandBuffer, frameNumber, imageIndex, [&](auto& setter) {
        setter.set("view", packet.camera->getViewMatrix());
        setter.set("projection", packet.camera->getProjectionMatrix());
    });
    commandBuffer.execute(DrawCommand{ .vertexCount = 6u });
}

}  // namespace sl

#include "GridRenderPass.hh"

#include "starlight/renderer/factories/ShaderFactory.hh"

namespace sl {

GridRenderPass::GridRenderPass(Renderer& renderer, const Vec2<f32>& viewportOffset) :
    RenderPass(
      renderer, ShaderFactory::get().load("Builtin.Shader.Grid"), viewportOffset,
      "GridRenderPass"
    ) {}

RenderPassBackend::Properties GridRenderPass::createProperties(
  bool hasPreviousPass, [[maybe_unused]] bool hasNextPass
) {
    return createDefaultProperties(
      Attachment::swapchainColor | Attachment::depth,
      hasPreviousPass ? ClearFlags::none : ClearFlags::depth
    );
}

void GridRenderPass::render(
  RenderPacket& packet, CommandBuffer& commandBuffer, u32 imageIndex
) {
    m_shader->setGlobalUniforms(
      commandBuffer, imageIndex,
      [&](Shader::UniformProxy& proxy) {
          proxy.set("view", packet.camera->getViewMatrix());
          proxy.set("projection", packet.camera->getProjectionMatrix());
      }
    );
    commandBuffer.execute(DrawCommand{ .vertexCount = 6u });
}

}  // namespace sl

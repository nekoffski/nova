#include "GridRenderView.hh"

#include "starlight/core/window/Window.hh"

#include "starlight/renderer/gpu/vulkan/VKCommandBuffer.hh"

namespace sl {

GridRenderView::GridRenderView(const Vec2<f32>& viewportOffset) :
    RenderView("GridRenderView", viewportOffset),
    m_shader(ShaderFactory::get().load("Builtin.Shader.Grid")) {}

RenderPass::Properties GridRenderView::generateRenderPassProperties(
  RendererBackend& renderer, [[maybe_unused]] RenderPass::ChainFlags chainFlags
) {
    return generateDefaultRenderPassProperties(
      renderer, Attachment::swapchainColor | Attachment::depth
    );
}

void GridRenderView::init(
  [[maybe_unused]] RendererBackend& backend, RenderPass& renderPass
) {
    m_shader->createPipeline(renderPass);
}

void GridRenderView::render(
  RendererBackend& renderer, RenderPacket& packet,
  [[maybe_unused]] const RenderProperties& properties,
  [[maybe_unused]] float deltaTime, CommandBuffer& commandBuffer, u8 imageIndex
) {
    m_shader->use(commandBuffer);
    m_shader->setGlobalUniforms(
      commandBuffer, imageIndex,
      [&](Shader::UniformProxy& proxy) {
          proxy.set("view", packet.camera->getViewMatrix());
          proxy.set("projection", packet.camera->getProjectionMatrix());
      }
    );

    // JUST TEMP: waiting for renderer refactor
    vkCmdDraw(
      static_cast<vk::VKCommandBuffer&>(commandBuffer).getHandle(), 6, 1, 0, 0
    );
}

}  // namespace sl

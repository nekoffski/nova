#include "RenderPass.hh"

#include <fmt/core.h>

#include "starlight/renderer/Renderer.hh"

namespace sl {

RenderPassBase::RenderPassBase(
  Renderer& renderer, const Vec2<f32>& viewportOffset,
  std::optional<std::string> name
) :
    m_renderer(renderer), m_viewportOffset(viewportOffset),
    name(name.value_or(fmt::format("RenderPass_{}", getId()))) {}

Rect2<u32> RenderPassBase::getViewport() {
    auto framebufferSize = m_renderer.getWindow().getFramebufferSize();

    Vec2<u32> viewportOffset{
        static_cast<u32>(framebufferSize.x * m_viewportOffset.x), 0u
    };

    return Rect2<u32>{
        viewportOffset,
        Vec2<u32>(
          framebufferSize.x - viewportOffset.x,
          static_cast<u32>((1.0f - m_viewportOffset.y) * framebufferSize.y)
        )
    };
}

RenderPassBackend::Properties RenderPassBase::createDefaultProperties(
  Attachment attachments, ClearFlags clearFlags, RenderPassBackend::Type type
) {
    RenderPassBackend::Properties props;

    props.clearColor = Vec4<f32>{ 0.0f };
    props.clearFlags = clearFlags;
    props.rect       = getViewport();
    props.type       = type;

    auto& swapchain = m_renderer.getSwapchain();

    const auto swapchainImageCount = swapchain.getImageCount();
    props.renderTargets.reserve(swapchainImageCount);

    RenderTarget renderTarget;

    for (u8 i = 0; i < swapchainImageCount; ++i) {
        renderTarget.depthAttachment = nullptr;
        renderTarget.colorAttachment = nullptr;

        if (isFlagEnabled(attachments, Attachment::swapchainColor))
            renderTarget.colorAttachment = swapchain.getImage(i);

        if (isFlagEnabled(attachments, Attachment::depth))
            renderTarget.depthAttachment = swapchain.getDepthBuffer();

        props.renderTargets.push_back(renderTarget);
    }

    return props;
}

RenderPass::RenderPass(
  Renderer& renderer, ResourceRef<Shader> shader, const Vec2<f32>& viewportOffset,
  std::optional<std::string> name
) : RenderPassBase(renderer, viewportOffset, name), m_shader(shader) {}

void RenderPass::run(
  RenderPacket& packet, CommandBuffer& commandBuffer, u32 imageIndex, u64 frameNumber
) {
    const auto viewport = getViewport();
    commandBuffer.execute(SetViewportCommand{
      .offset = viewport.offset,
      .size   = viewport.size,
    });

    m_renderPassBackend->run(
      commandBuffer, imageIndex,
      [&](CommandBuffer& commandBuffer, u32 imageIndex) {
          m_pipeline->bind(commandBuffer);
          render(packet, commandBuffer, imageIndex, frameNumber);
      }
    );
}

void RenderPass::init(bool hasPreviousPass, bool hasNextPass) {
    const auto props = createProperties(hasPreviousPass, hasNextPass);
    auto& device     = m_renderer.getDevice();

    m_renderPassBackend.clear();
    m_pipeline.clear();

    m_renderPassBackend =
      device.createRenderPassBackend(props, hasPreviousPass, hasNextPass);
    m_pipeline = device.createPipeline(*m_shader, *m_renderPassBackend);
}

void RenderPass::drawMesh(Mesh& mesh, CommandBuffer& commandBuffer) {
    const auto& memoryLayout = mesh.getMemoryLayout();

    commandBuffer.execute(BindVertexBufferCommand{
      .buffer = m_renderer.getVertexBuffer(),
      .offset = memoryLayout.vertexBufferRange.offset,
    });

    commandBuffer.execute(BindIndexBufferCommand{
      .buffer = m_renderer.getIndexBuffer(),
      .offset = memoryLayout.indexBufferRange.offset,
    });

    commandBuffer.execute(DrawIndexedCommand{
      .indexCount = static_cast<u32>(memoryLayout.indexCount),
    });
}

}  // namespace sl

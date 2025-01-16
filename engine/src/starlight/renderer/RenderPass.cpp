#include "RenderPass.hh"

#include <fmt/core.h>

#include "starlight/renderer/Renderer.hh"

namespace sl {

RenderPass::RenderPass(
  Renderer& renderer, ResourceRef<Shader> shader, const Vec2<f32>& viewportOffset,
  std::optional<std::string> name
) :
    m_renderer(renderer), m_shader(shader), m_viewportOffset(viewportOffset),
    name(name.value_or(fmt::format("RenderPass_{}", getId()))) {}

void RenderPass::run(
  RenderPacket& packet, CommandBuffer& commandBuffer, u32 imageIndex
) {
    const auto viewport = getViewport();
    commandBuffer.execute(SetViewportCommand{
      .offset = viewport.offset,
      .size   = viewport.size,
    });

    m_renderPassBackend->run(
      commandBuffer, imageIndex,
      [&](CommandBuffer& commandBuffer, u32 imageIndex) {
          m_shader->bindPipeline(*m_pipeline);
          m_pipeline->bind(commandBuffer);
          render(packet, commandBuffer, imageIndex);
      }
    );
}

void RenderPass::init(bool hasPreviousPass, bool hasNextPass) {
    m_renderPassBackend.clear();
    const auto props = createProperties(hasPreviousPass, hasNextPass);
    auto& device     = m_renderer.getDevice();

    m_renderPassBackend =
      device.createRenderPassBackend(props, hasPreviousPass, hasNextPass);
    m_pipeline = device.createPipeline(*m_shader, *m_renderPassBackend);
}

Rect2<u32> RenderPass::getViewport() {
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

RenderPassBackend::Properties RenderPass::createDefaultProperties(
  Attachment attachments, ClearFlags clearFlags
) {
    RenderPassBackend::Properties props;

    props.clearColor = Vec4<f32>{ 0.0f };
    props.clearFlags = clearFlags;
    props.rect       = getViewport();

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

}  // namespace sl

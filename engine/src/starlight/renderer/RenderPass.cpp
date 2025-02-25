#include "RenderPass.hh"

#include <fmt/core.h>

#include "starlight/core/LazyEvaluator.hh"
#include "starlight/renderer/Renderer.hh"
#include "starlight/window/Window.hh"

namespace sl {

RenderPassBase::RenderPassBase(
  Renderer& renderer, const Vec2<f32>& viewportOffset,
  std::optional<std::string> name
) : NamedResource(name), m_renderer(renderer), m_viewportOffset(viewportOffset) {}

Rect2<u32> RenderPassBase::getViewport() {
    auto framebufferSize = Window::get().getFramebufferSize();

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

RenderPassBackend::Properties RenderPassBase::generateRenderPassProperties(
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

Pipeline::Properties RenderPassBase::createPipelineProperties() {
    return Pipeline::Properties::createDefault();
}

RenderPass::RenderPass(
  Renderer& renderer, SharedPointer<Shader> shader, const Vec2<f32>& viewportOffset,
  std::optional<std::string> name
) :
    RenderPassBase(renderer, viewportOffset, name), m_shader(shader),
    m_shaderDataBinder(ShaderDataBinder::create(*m_shader)) {}

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
    const auto props = createRenderPassProperties(hasPreviousPass, hasNextPass);

    m_renderPassBackend.clear();
    m_pipeline.clear();

    m_renderPassBackend =
      RenderPassBackend::create(props, hasPreviousPass, hasNextPass);
    m_pipeline =
      Pipeline::create(*m_shader, *m_renderPassBackend, createPipelineProperties());
}

void RenderPass::setLocalUniforms(
  CommandBuffer& commandBuffer, u64 frameNumber, u32 id, u32 imageIndex,
  ShaderDataBinder::UniformCallback&& callback
) {
    m_shaderDataBinder->setLocalUniforms(
      *m_pipeline, commandBuffer, frameNumber, id, imageIndex, std::move(callback)
    );
}

u32 RenderPass::getLocalDescriporSetId(u32 id) {
    const auto [it, _] = m_localDescriptorSets.try_emplace(
      id,
      lazyEvaluate([&] { return m_shaderDataBinder->acquireLocalDescriptorSet(); })
    );
    return it->second;
}

void RenderPass::setGlobalUniforms(
  CommandBuffer& commandBuffer, u64 frameNumber, u32 imageIndex,
  ShaderDataBinder::UniformCallback&& callback
) {
    m_shaderDataBinder->setGlobalUniforms(
      *m_pipeline, commandBuffer, frameNumber, imageIndex, std::move(callback)
    );
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

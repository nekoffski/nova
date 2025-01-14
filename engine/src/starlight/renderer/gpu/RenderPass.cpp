#include "RenderPass.hh"

#include <fmt/core.h>

#include "RenderTarget.hh"
#include "starlight/renderer/Renderer.hh"
#include "starlight/core/utils/Enum.hh"

#ifdef SL_USE_VK
#include "starlight/renderer/gpu/vulkan/VKRenderPass.hh"
#include "starlight/renderer/gpu/vulkan/VKRendererBackend.hh"
#endif

namespace sl {

namespace v2 {

RenderPass::RenderPass(
  Renderer& renderer, const Vec2<f32>& viewportOffset,
  std::optional<std::string> name
) :
    m_renderer(renderer), m_viewportOffset(viewportOffset),
    name(name.value_or(fmt::format("RenderPass_{}", getId()))) {}

void RenderPass::run(
  RenderPacket& packet, CommandBuffer& commandBuffer, u8 imageIndex
) {
    m_renderPassImpl->begin(commandBuffer, imageIndex);
    render(packet, commandBuffer, imageIndex);
    m_renderPassImpl->end(commandBuffer);
}

void RenderPass::init(bool hasPreviousPass, bool hasNextPass) {
    m_renderPassImpl.clear();
    const auto props = createProperties(hasPreviousPass, hasNextPass);
    m_renderPassImpl = m_renderer.getDevice().createRenderPass(props);
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

RenderPass::Properties RenderPass::createDefaultProperties(
  RenderPass::Attachment attachments, RenderPass::ClearFlags clearFlags
) {
    RenderPass::Properties props;

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

}  // namespace v2

OwningPtr<RenderPass> RenderPass::create(
  RendererBackend& renderer, const Properties& props, ChainFlags chainFlags
) {
#ifdef SL_USE_VK
    auto& vkRenderer = static_cast<vk::VKRendererBackend&>(renderer);

    LOG_TRACE("Creating instance of vulkan render pass");

    return createOwningPtr<vk::VKRenderPass>(
      vkRenderer.getContext(), vkRenderer.getLogicalDevice(),
      vkRenderer.getSwapchain(), props, chainFlags
    );
#else
    FATAL_ERROR("Could not find renderer backend implementation");
#endif
}

RenderPass::RenderPass(const Properties& props) : m_props(props) {}

void RenderPass::setClearColor(const Vec4<f32>& color) {
    m_props.clearColor = color;
}

void RenderPass::setRect(const Rect2<u32>& extent) { m_props.rect = extent; }

void RenderPass::setRectSize(const Vec2<u32>& size) { m_props.rect.size = size; }

const RenderPass::Properties& RenderPass::getProperties() const { return m_props; }

}  // namespace sl

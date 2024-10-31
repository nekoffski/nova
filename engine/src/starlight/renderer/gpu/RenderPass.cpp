#include "RenderPass.hh"

#include "RenderTarget.hh"

#ifdef SL_USE_VK
#include "starlight/renderer/gpu/vulkan/VKRenderPass.hh"
#include "starlight/renderer/gpu/vulkan/VKRendererBackend.hh"
#endif

namespace sl {

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

void RenderPass::setRect(const Rect2u32& extent) { m_props.rect = extent; }

void RenderPass::setRectSize(const Vec2<u32>& size) { m_props.rect.size = size; }

}  // namespace sl

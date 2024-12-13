#include "RenderView.hh"

#include "starlight/core/window/Window.hh"
#include "starlight/core/math/Core.hh"

namespace sl {

RenderView::RenderView(const std::string& name, const Vec2<f32>& viewportOffset) :
    name(name), m_viewportOffset(viewportOffset) {}

RenderPass::Properties RenderView::getDefaultRenderPassProperties(
  RendererBackend& renderer, Attachment attachments,
  RenderPass::ClearFlags clearFlags
) const {
    RenderPass::Properties props;

    props.clearColor = Vec4<f32>{ 0.0f };
    props.clearFlags = clearFlags;
    props.rect       = getViewport();

    const auto swapchainImageCount = renderer.getSwapchainImageCount();
    props.renderTargets.reserve(swapchainImageCount);

    RenderTarget renderTarget;
    renderTarget.size = Window::get().getFramebufferSize();

    props.includeDepthAttachment = isFlagEnabled(attachments, Attachment::depth);

    for (u8 i = 0; i < swapchainImageCount; ++i) {
        renderTarget.attachments.clear();
        if (isFlagEnabled(attachments, Attachment::swapchainColor))
            renderTarget.attachments.push_back(renderer.getSwapchainTexture(i));
        if (props.includeDepthAttachment)
            renderTarget.attachments.push_back(renderer.getDepthTexture());
        props.renderTargets.push_back(renderTarget);
    }

    return props;
}

Rect2<u32> RenderView::getViewport() const {
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

}  // namespace sl

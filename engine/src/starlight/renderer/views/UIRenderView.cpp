#include "UIRenderView.hh"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include "starlight/renderer/gpu/vulkan/VKCommandBuffer.hh"

namespace sl {

UIRenderView::UIRenderView(
  const FontsProperties& fontsProperties, UICallback&& callback
) : m_fontsProperties(fontsProperties), m_uiCallback(callback) {}

RenderPass::Properties UIRenderView::getRenderPassProperties(
  RendererBackend& renderer, [[maybe_unused]] RenderPass::ChainFlags chainFlags
) const {
    auto props       = getDefaultRenderPassProperties();
    props.clearColor = Vec4<f32>{ 0.0f };
    props.clearFlags =
      isFlagEnabled(chainFlags, RenderPass::ChainFlags::hasPrevious)
        ? RenderPass::ClearFlags::none
        : RenderPass::ClearFlags::color;

    const auto swapchainImageCount = renderer.getSwapchainImageCount();
    props.renderTargets.reserve(swapchainImageCount);

    RenderTarget renderTarget;
    renderTarget.size = props.rect.size;

    for (u8 i = 0; i < swapchainImageCount; ++i) {
        renderTarget.attachments = { renderer.getSwapchainTexture(i) };
        props.renderTargets.push_back(renderTarget);
    }

    props.includeDepthAttachment = false;
    return props;
}

void UIRenderView::init(RendererBackend& renderer, RenderPass& renderPass) {
    m_uiRenderer = UIRenderer::create(renderer, renderPass);
    m_uiRenderer->setStyle();

    std::transform(
      m_fontsProperties.begin(), m_fontsProperties.end(),
      std::back_inserter(m_fonts),
      [&](const auto& props) -> Font* { return m_uiRenderer->addFont(props); }
    );
}

void UIRenderView::render(
  [[maybe_unused]] RendererBackend& renderer,
  [[maybe_unused]] const RenderPacket& packet,
  [[maybe_unused]] const RenderProperties& props, [[maybe_unused]] float deltaTime,
  CommandBuffer& commandBuffer, [[maybe_unused]] u8 imageIndex
) {
    m_uiRenderer->render(commandBuffer, m_uiCallback);
}

}  // namespace sl

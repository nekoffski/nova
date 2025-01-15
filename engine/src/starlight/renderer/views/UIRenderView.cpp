#include "UIRenderView.hh"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include "starlight/renderer/gpu/vulkan/VKCommandBuffer.hh"

namespace sl {

UIRenderView::UIRenderView(
  const FontsProperties& fontsProperties, UICallback&& callback
) :
    RenderView("UIRenderView", { 0.0f, 0.0f }), m_fontsProperties(fontsProperties),
    m_uiCallback(callback) {}

RenderPass::Properties UIRenderView::generateRenderPassProperties(
  RendererBackend& renderer, RenderPass::ChainFlags chainFlags
) {
    const auto clearFlags =
      isFlagEnabled(chainFlags, RenderPass::ChainFlags::hasPrevious)
        ? RenderPass::ClearFlags::none
        : RenderPass::ClearFlags::color;
    return generateDefaultRenderPassProperties(
      renderer, Attachment::swapchainColor, clearFlags
    );
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
  [[maybe_unused]] RendererBackend& renderer, [[maybe_unused]] RenderPacket& packet,
  [[maybe_unused]] const RenderProperties& props, [[maybe_unused]] float deltaTime,
  CommandBuffer& commandBuffer, [[maybe_unused]] u32 imageIndex
) {
    m_uiRenderer->render(commandBuffer, m_uiCallback);
}

}  // namespace sl

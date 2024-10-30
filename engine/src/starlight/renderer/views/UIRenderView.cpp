#include "UIRenderView.hh"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include "starlight/renderer/gpu/vulkan/VKCommandBuffer.hh"

namespace sl {

UIRenderView::UIRenderView(const Properties& props, UICallback&& callback) :
    m_props(props), m_uiCallback(std::move(callback)) {}

void UIRenderView::init(
  RendererBackend& renderer, const InitProperties& initProperties
) {
    LOG_TRACE("Initializing UIRenderView");

    RenderPass::Properties renderPassProperties{
        .rect       = Rect2u32{ Vec2<u32>{ 0u, 0u }, initProperties.viewportSize },
        .clearColor = glm::vec4(0.0f),
        .clearFlags = RenderPass::clearNone,
        .hasPreviousPass = initProperties.hasPreviousView,
        .hasNextPass     = initProperties.hasNextView,
        .renderTargets   = {}
    };

    RenderTarget renderTarget{
        .size = initProperties.viewportSize, .attachments = {}
    };

    for (u8 i = 0; i < 3; ++i) {
        renderTarget.attachments = { renderer.getSwapchainTexture(i) };
        renderPassProperties.renderTargets.push_back(renderTarget);
    }

    m_renderPass = RenderPass::create(renderer, renderPassProperties);
    m_uiRenderer = UIRenderer::create(renderer, *m_renderPass);

    m_uiRenderer->setStyle();

    for (const auto& fontProperties : m_props.fonts)
        m_fonts.push_back(m_uiRenderer->addFont(fontProperties));

    LOG_TRACE("UIRenderView initialized");
}

void UIRenderView::render(
  RendererBackend& renderer, [[maybe_unused]] const RenderPacket& packet,
  [[maybe_unused]] const RenderProperties& properties,
  [[maybe_unused]] float deltaTime
) {
    m_renderPass->run(
      renderer.getCommandBuffer(), renderer.getImageIndex(),
      [&](CommandBuffer& commandBuffer, [[maybe_unused]] u32) {
          m_uiRenderer->render(commandBuffer, m_uiCallback);
      }
    );
}

void UIRenderView::onViewportResize(
  RendererBackend& renderer, Vec2<u32> viewportSize
) {
    // TODO: get swapchain images count from backend
    std::vector<RenderTarget> renderTargets;
    renderTargets.reserve(3);

    RenderTarget renderTarget{ .size = viewportSize, .attachments = {} };

    for (u8 i = 0; i < 3; ++i) {
        renderTarget.attachments = { renderer.getSwapchainTexture(i) };
        renderTargets.push_back(renderTarget);
    }
    m_renderPass->regenerateRenderTargets(renderTargets);
    m_renderPass->setRectSize(viewportSize);
}

}  // namespace sl
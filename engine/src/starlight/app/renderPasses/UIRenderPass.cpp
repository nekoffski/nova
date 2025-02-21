#include "UIRenderPass.hh"

#include "starlight/renderer/Renderer.hh"

namespace sl {

sl::UIRenderPass::UIRenderPass(Renderer& renderer, UI& ui) :
    RenderPassBase(renderer, { 0.0f, 0.0f }, "UIRenderPass"), m_ui(ui) {}

void UIRenderPass::init(bool hasPreviousPass, bool hasNextPass) {
    const auto props = createRenderPassProperties(hasPreviousPass, hasNextPass);

    m_renderPassBackend.clear();
    m_renderPassBackend =
      RenderPassBackend::create(props, hasPreviousPass, hasNextPass);
}

void UIRenderPass::run(
  [[maybe_unused]] RenderPacket& packet, CommandBuffer& commandBuffer,
  u32 imageIndex, [[maybe_unused]] u64 frameNumber
) {
    const auto viewport = getViewport();
    commandBuffer.execute(SetViewportCommand{
      .offset = viewport.offset,
      .size   = viewport.size,
    });

    m_renderPassBackend->run(commandBuffer, imageIndex, [&] { m_ui.render(); });
}

RenderPassBackend::Properties UIRenderPass::createRenderPassProperties(
  bool hasPreviousPass, [[maybe_unused]] bool hasNextPass
) {
    return generateRenderPassProperties(
      Attachment::swapchainColor,
      hasPreviousPass ? ClearFlags::none : ClearFlags::color,
      RenderPassBackend::Type::ui
    );
}

}  // namespace sl

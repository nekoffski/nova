#include "UIRenderPass.hh"

#include "starlight/renderer/Renderer.hh"

namespace sl {

sl::UIRenderPass::UIRenderPass(Renderer& renderer, UI& ui) :
    RenderPassBase(renderer, { 0.0f, 0.0f }, "UIRenderPass"), m_ui(ui) {}

void UIRenderPass::init(bool hasPreviousPass, bool hasNextPass) {
    const auto props = createProperties(hasPreviousPass, hasNextPass);
    auto& device     = m_renderer.getDevice();

    m_renderPassBackend =
      device.createRenderPassBackend(props, hasPreviousPass, hasNextPass);
}

void UIRenderPass::run(
  RenderPacket& packet, CommandBuffer& commandBuffer, u32 imageIndex
) {
    const auto viewport = getViewport();
    commandBuffer.execute(SetViewportCommand{
      .offset = viewport.offset,
      .size   = viewport.size,
    });

    auto render = [&]([[maybe_unused]] CommandBuffer&, [[maybe_unused]] u32) {
        m_ui.render();
    };
    m_renderPassBackend->run(commandBuffer, imageIndex, render);
}

RenderPassBackend::Properties UIRenderPass::createProperties(
  bool hasPreviousPass, bool hasNextPass
) {
    return createDefaultProperties(
      Attachment::swapchainColor,
      hasPreviousPass ? ClearFlags::none : ClearFlags::color,
      RenderPassBackend::Type::ui
    );
}

}  // namespace sl

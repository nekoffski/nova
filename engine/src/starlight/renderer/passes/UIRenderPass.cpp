// #include "UIRenderPass.hh"

// #include <imgui.h>
// #include <backends/imgui_impl_glfw.h>
// #include <backends/imgui_impl_vulkan.h>

// #include "starlight/renderer/gpu/vulkan/VulkanCommandBuffer.hh"

// namespace sl {

// UIRenderPass::UIRenderPass(
//   const FontsProperties& fontsProperties, UICallback&& callback
// ) :
//     RenderView("UIRenderPass", { 0.0f, 0.0f }),
//     m_fontsProperties(fontsProperties), m_uiCallback(callback) {}

// RenderPassBackend::Properties UIRenderPass::generateRenderPassProperties(
//   RendererBackend& renderer, RenderPass::ChainFlags chainFlags
// ) {
//     const auto clearFlags =
//       isFlagEnabled(chainFlags, RenderPass::ChainFlags::hasPrevious)
//         ? RenderPass::ClearFlags::none
//         : RenderPass::ClearFlags::color;
//     return generateDefaultRenderPassProperties(
//       renderer, Attachment::swapchainColor, clearFlags
//     );
// }

// void UIRenderPass::init(RendererBackend& renderer, RenderPass& renderPass) {
//     m_uiRenderer = UIRenderer::create(renderer, renderPass);
//     m_uiRenderer->setStyle();

//     std::transform(
//       m_fontsProperties.begin(), m_fontsProperties.end(),
//       std::back_inserter(m_fonts),
//       [&](const auto& props) -> Font* { return m_uiRenderer->addFont(props); }
//     );
// }

// void UIRenderPass::render(
//   [[maybe_unused]] RendererBackend& renderer, [[maybe_unused]] RenderPacket&
//   packet,
//   [[maybe_unused]] const RenderProperties& props, [[maybe_unused]] float
//   deltaTime, CommandBuffer& commandBuffer, [[maybe_unused]] u32 imageIndex
// ) {
//     m_uiRenderer->render(commandBuffer, m_uiCallback);
// }

// }  // namespace sl

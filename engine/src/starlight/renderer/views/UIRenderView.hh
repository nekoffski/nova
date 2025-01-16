// #pragma once

// #include <functional>

// #include "starlight/core/memory/Memory.hh"

// #include "RenderView.hh"

// #include "starlight/renderer/RenderPass.hh"
// #include "starlight/renderer/gpu/UIRenderer.hh"

// namespace sl {

// class UIRenderView : public RenderView {
// public:
//     using FontsProperties = std::vector<Font::Properties>;
//     using UICallback      = std::function<void()>;

//     explicit UIRenderView(const FontsProperties& fonts, UICallback&& callback);

//     RenderPassBackend::Properties generateRenderPassProperties(
//       RendererBackend& renderer, RenderPass::ChainFlags chainFlags
//     ) override;

//     void init(RendererBackend& renderer, RenderPass& renderPass) override;

//     void render(
//       RendererBackend& renderer, RenderPacket& packet, const RenderProperties&
//       props, float deltaTime, CommandBuffer& commandBuffer, u32 imageIndex
//     ) override;

// private:
//     FontsProperties m_fontsProperties;
//     UICallback m_uiCallback;
//     OwningPtr<UIRenderer> m_uiRenderer;
//     std::vector<Font*> m_fonts;
// };

// }  // namespace sl

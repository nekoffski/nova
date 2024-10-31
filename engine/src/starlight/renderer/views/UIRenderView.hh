// #pragma once

// #include <functional>

// #include "starlight/core/memory/Memory.hh"

// #include "RenderView.hh"

// #include "starlight/renderer/gpu/RenderPass.hh"
// #include "starlight/renderer/gpu/UIRenderer.hh"

// namespace sl {

// class UIRenderView : public RenderView {
// public:
//     struct Properties {
//         std::vector<Font::Properties> fonts;
//     };

//     using UICallback = std::function<void()>;

//     explicit UIRenderView(const Properties& props, UICallback&& callback);

//     void init(RendererBackend& renderer, const InitProperties& initProperties)
//       override;

//     void render(
//       RendererBackend& renderer, const RenderPacket& packet,
//       const RenderProperties& properties, float deltaTime
//     ) override;

//     void onViewportResize(RendererBackend& renderer, Vec2<u32> viewportSize)
//       override;

// private:
//     Properties m_props;
//     UICallback m_uiCallback;
//     OwningPtr<UIRenderer> m_uiRenderer;

//     std::vector<Font*> m_fonts;
// };

// }  // namespace sl
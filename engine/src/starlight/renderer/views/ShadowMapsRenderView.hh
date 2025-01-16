// #pragma once

// #include <functional>

// #include "RenderView.hh"

// #include "starlight/renderer/RenderPass.hh"
// #include "starlight/renderer/gpu/Shader.hh"

// namespace sl {

// class ShadowMapsRenderView : public RenderView {
// public:
//     explicit ShadowMapsRenderView(const Vec2<f32>& viewportOffset);

//     RenderPassBackend::Properties generateRenderPassProperties(
//       RendererBackend& renderer, RenderPass::ChainFlags chainFlags
//     ) override;

//     void init(RendererBackend& renderer, RenderPass& renderPass) override;

//     void render(
//       RendererBackend& renderer, RenderPacket& packet, const RenderProperties&
//       props, float deltaTime, CommandBuffer& commandBuffer, u32 imageIndex
//     ) override;

//     Rect2<u32> getViewport() const override;

// private:
//     ResourceRef<Shader> m_shader;
//     std::vector<OwningPtr<Texture>> m_shadowMaps;
// };

// }  // namespace sl

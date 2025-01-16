// #pragma once

// #include <functional>

// #include "RenderView.hh"

// #include "starlight/renderer/RenderPass.hh"
// #include "starlight/renderer/gpu/Shader.hh"

// namespace sl {

// class WorldRenderView : public RenderView {
// public:
//     explicit WorldRenderView(
//       const Vec2<f32>& viewportOffset, ResourceRef<Shader> shader
//     );

//     RenderPassBackend::Properties generateRenderPassProperties(
//       RendererBackend& renderer, RenderPass::ChainFlags chainFlags
//     ) override;

//     void init(RendererBackend& renderer, RenderPass& renderPass) override;

//     void render(
//       RendererBackend& renderer, RenderPacket& packet, const RenderProperties&
//       props, float deltaTime, CommandBuffer& commandBuffer, u32 imageIndex
//     ) override;

// private:
//     ResourceRef<Shader> m_shader;
// };

// }  // namespace sl

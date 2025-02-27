// #pragma once

// #include "RenderView.hh"
// #include "starlight/renderer/RenderPass.hh"
// #include "starlight/renderer/gpu/Shader.hh"

// namespace sl {

// class LightsDebugRenderView : public RenderView {
// public:
//     explicit LightsDebugRenderView(const Vec2<f32>& viewportOffset);

//     RenderPassBackend::Properties generateRenderPassProperties(
//       RendererBackend& renderer, RenderPass::ChainFlags chainFlags
//     ) override;

//     void init(RendererBackend& renderer, RenderPass& renderPass) override;

//     void render(
//       RendererBackend& renderer, RenderPacket& packet, const RenderProperties&
//       props, float deltaTime, CommandBuffer& commandBuffer, u32 imageIndex
//     ) override;

// private:
//     SharedPtr<Shader> m_shader;
//     Mesh* m_mesh;
// };

// }  // namespace sl
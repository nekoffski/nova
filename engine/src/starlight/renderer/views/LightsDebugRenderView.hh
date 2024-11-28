#pragma once

#include "RenderView.hh"

#include "starlight/renderer/gpu/RenderPass.hh"
#include "starlight/renderer/gpu/Shader.hh"

namespace sl {

class LightsDebugRenderView : public RenderView {
public:
    explicit LightsDebugRenderView();

    RenderPass::Properties getRenderPassProperties(
      RendererBackend& renderer, RenderPass::ChainFlags chainFlags
    ) const override;

    void init(RendererBackend& renderer, RenderPass& renderPass) override;
    void preRender(RendererBackend& renderer) override;

    void render(
      RendererBackend& renderer, const RenderPacket& packet,
      const RenderProperties& props, float deltaTime, CommandBuffer& commandBuffer,
      u8 imageIndex
    ) override;

private:
    ResourceRef<Shader> m_shader;
    Mesh* m_mesh;
};

}  // namespace sl
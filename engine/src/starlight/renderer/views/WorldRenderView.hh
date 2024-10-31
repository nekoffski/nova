#pragma once

#include <functional>

#include "RenderView.hh"

#include "starlight/renderer/gpu/RenderPass.hh"
#include "starlight/renderer/gpu/Shader.hh"

namespace sl {

class WorldRenderView : public RenderView {
public:
    explicit WorldRenderView(Shader* shader);

    RenderPass::Properties getRenderPassProperties(
      RendererBackend& renderer, RenderPass::ChainFlags chainFlags
    ) const override;

    void init(RenderPass& renderPass) override;
    void preRender(RendererBackend& renderer) override;

    void render(
      RendererBackend& renderer, const RenderPacket& packet,
      const RenderProperties& props, float deltaTime, CommandBuffer& commandBuffer,
      u8 imageIndex
    ) override;

private:
    Shader* m_shader;
};

}  // namespace sl
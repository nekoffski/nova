#pragma once

#include <functional>

#include "RenderView.hh"

#include "starlight/renderer/gpu/RenderPass.hh"
#include "starlight/renderer/gpu/Shader.hh"

namespace sl {

class GridRenderView : public RenderView {
public:
    explicit GridRenderView(const Vec2<f32>& viewportOffset);

    RenderPass::Properties generateRenderPassProperties(
      RendererBackend& renderer, RenderPass::ChainFlags chainFlags
    ) override;

    void init(RendererBackend& renderer, RenderPass& renderPass) override;

    void render(
      RendererBackend& renderer, RenderPacket& packet, const RenderProperties& props,
      float deltaTime, CommandBuffer& commandBuffer, u8 imageIndex
    ) override;

private:
    ResourceRef<Shader> m_shader;
};

}  // namespace sl

#pragma once

#include "starlight/renderer/RenderPass.hh"

namespace sl {

class GridRenderPass : public RenderPass {
public:
    explicit GridRenderPass(Renderer& renderer, const Vec2<f32>& viewportOffset);

private:
    RenderPassBackend::Properties createProperties(
      bool hasPreviousPass, bool hasNextPass
    ) override;

    void render(RenderPacket& packet, CommandBuffer& commandBuffer, u32 imageIndex)
      override;
};

}  // namespace sl

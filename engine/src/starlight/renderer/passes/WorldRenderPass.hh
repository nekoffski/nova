#pragma once

#include "starlight/renderer/RenderPass.hh"

namespace sl {

class WorldRenderPass : public RenderPass {
public:
    explicit WorldRenderPass(Renderer& renderer);

private:
    RenderPassBackend::Properties createProperties(
      bool hasPreviousPass, bool hasNextPass
    ) override;

    void render(RenderPacket& packet, CommandBuffer& commandBuffer, u32 imageIndex)
      override;
};

}  // namespace sl

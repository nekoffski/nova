#pragma once

#include "starlight/renderer/RenderPass.hh"

namespace sl {

class ShadowMapsRenderPass : public RenderPass {
public:
    explicit ShadowMapsRenderPass(Renderer& renderer);

private:
    RenderPassBackend::Properties createRenderPassProperties(
      bool hasPreviousPass, bool hasNextPass
    ) override;

    void render(
      RenderPacket& packet, CommandBuffer& commandBuffer, u32 imageIndex,
      u64 frameNumber
    ) override;

    Rect2<u32> getViewport() override;

    std::vector<UniquePointer<Texture>> m_shadowMaps;
};

}  // namespace sl

#pragma once

#include "starlight/renderer/RenderPass.hh"

namespace sl {

class ShadowMapsRenderPass : public RenderPass {
public:
    explicit ShadowMapsRenderPass(Renderer& renderer);

private:
    RenderPassBackend::Properties createProperties(
      bool hasPreviousPass, bool hasNextPass
    ) override;

    void render(RenderPacket& packet, CommandBuffer& commandBuffer, u32 imageIndex)
      override;

    Rect2<u32> getViewport() override;

    std::vector<OwningPtr<Texture>> m_shadowMaps;
};

}  // namespace sl

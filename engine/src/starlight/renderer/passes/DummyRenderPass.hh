#pragma once

#include "starlight/renderer/gpu/RenderPass.hh"

namespace sl {

class DummyRenderPass : public v2::RenderPass {
public:
    explicit DummyRenderPass(Renderer& renderer) : RenderPass(renderer) {}

private:
    Properties createProperties(
      bool hasPreviousPass, [[maybe_unused]] bool hasNextPass
    ) override {
        auto clearFlags = ClearFlags::depth | ClearFlags::stencil;
        if (not hasPreviousPass) clearFlags |= RenderPass::ClearFlags::color;

        return createDefaultProperties(
          Attachment::swapchainColor | Attachment::depth, clearFlags
        );
    }

    void render(
      RenderPacket& packet, v2::CommandBuffer& commandBuffer, u32 imageIndex
    ) override {}
};

}  // namespace sl

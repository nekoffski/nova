#pragma once

#include "starlight/renderer/RenderPass.hh"
#include "starlight/renderer/factories/ShaderFactory.hh"

namespace sl {

class DummyRenderPass : public RenderPass {
public:
    explicit DummyRenderPass(Renderer& renderer
    ) : RenderPass(renderer, ShaderFactory::get().load("Builtin.Shader.Material")) {}

private:
    RenderPassBackend::Properties createProperties(
      bool hasPreviousPass, [[maybe_unused]] bool hasNextPass
    ) override {
        auto clearFlags = ClearFlags::depth | ClearFlags::stencil;
        if (not hasPreviousPass) clearFlags |= ClearFlags::color;

        auto props = createDefaultProperties(
          Attachment::swapchainColor | Attachment::depth, clearFlags
        );

        props.clearColor = Vec4<f32>{ 1.0f, 0.0f, 0.0f, 1.0f };

        return props;
    }

    void render(RenderPacket& packet, CommandBuffer& commandBuffer, u32 imageIndex)
      override {}
};

}  // namespace sl

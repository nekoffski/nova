#pragma once

#include "starlight/renderer/RenderPass.hh"

namespace sl {

struct UI {
    virtual void render() = 0;
};

class UIRenderPass final : public RenderPassBase {
public:
    explicit UIRenderPass(Renderer& renderer, UI& ui);

private:
    void init(bool hasPreviousPass, bool hasNextPass) override;

    void run(RenderPacket& packet, CommandBuffer& commandBuffer, u32 imageIndex)
      override;

    RenderPassBackend::Properties createProperties(
      bool hasPreviousPass, bool hasNextPass
    ) override;

    UI& m_ui;
};

}  // namespace sl

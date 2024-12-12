#pragma once

#include "starlight/renderer/gpu/RendererBackend.hh"
#include "starlight/renderer/gpu/RenderPass.hh"
#include "starlight/renderer/RenderPacket.hh"
#include "starlight/renderer/RenderProperties.hh"

namespace sl {

class RenderView {
public:
    enum class Attachment : u8 { swapchainColor = 0b01, depth = 0b10 };

    RenderView(const std::string& name, const Vec2<f32>& viewportOffset);

    virtual ~RenderView() = default;

    virtual RenderPass::Properties getRenderPassProperties(
      RendererBackend& renderer, RenderPass::ChainFlags chainFlags
    ) const = 0;

    virtual void init(
      [[maybe_unused]] RendererBackend& renderer,
      [[maybe_unused]] RenderPass& renderPass
    ) {}

    virtual void render(
      RendererBackend& renderer, const RenderPacket& packet,
      const RenderProperties& props, float deltaTime, CommandBuffer& commandBuffer,
      u8 imageIndex
    ) = 0;

    Rect2<u32> getViewport() const;

    const std::string name;

protected:
    RenderPass::Properties getDefaultRenderPassProperties(
      RendererBackend& renderer, Attachment attachments,
      RenderPass::ClearFlags clearFlags = RenderPass::ClearFlags::none
    ) const;

    Vec2<f32> m_viewportOffset;
};

constexpr void enableBitOperations(RenderView::Attachment);

}  // namespace sl

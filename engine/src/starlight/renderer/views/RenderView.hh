#pragma once

#include "starlight/renderer/gpu/RendererBackend.hh"
#include "starlight/renderer/gpu/RenderPass.hh"
#include "starlight/renderer/RenderPacket.hh"
#include "starlight/renderer/RenderProperties.hh"

namespace sl {

struct RenderView {
    virtual ~RenderView() = default;

    virtual RenderPass::Properties getRenderPassProperties(
      RendererBackend& renderer, RenderPass::ChainFlags chainFlags
    ) const = 0;

    virtual void init(
      [[maybe_unused]] RendererBackend& renderer,
      [[maybe_unused]] RenderPass& renderPass
    ) {}
    virtual void preRender([[maybe_unused]] RendererBackend& renderer) {}

    virtual void render(
      RendererBackend& renderer, const RenderPacket& packet,
      const RenderProperties& props, float deltaTime, CommandBuffer& commandBuffer,
      u8 imageIndex
    ) = 0;

    static RenderPass::Properties getDefaultRenderPassProperties();
};

}  // namespace sl

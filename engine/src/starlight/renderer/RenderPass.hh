#pragma once

#include <vector>
#include <optional>

#include "starlight/core/Core.hh"
#include "starlight/core/math/Core.hh"
#include "starlight/core/utils/Enum.hh"
#include "starlight/core/utils/Id.hh"
#include "starlight/core/utils/Resource.hh"
#include "starlight/renderer/RenderPacket.hh"

#include "gpu/Texture.hh"
#include "gpu/CommandBuffer.hh"
#include "gpu/RenderPassBackend.hh"
#include "gpu/Pipeline.hh"

#include "fwd.hh"
#include "starlight/renderer/fwd.hh"
#include "starlight/renderer/Core.hh"

namespace sl {

class RenderPass : public NonMovable, public Identificable<RenderPass> {
public:
    explicit RenderPass(
      Renderer& renderer, ResourceRef<Shader> shader,
      const Vec2<f32>& viewportOffset = { 0.0f, 0.0f },
      std::optional<std::string> name = {}
    );

    void init(bool hasPreviousPass, bool hasNextPass);
    void run(RenderPacket& packet, CommandBuffer& commandBuffer, u32 imageIndex);

protected:
    RenderPassBackend::Properties createDefaultProperties(
      Attachment attachments, ClearFlags clearFlags = ClearFlags::none
    );

    virtual Rect2<u32> getViewport();
    virtual RenderPassBackend::Properties createProperties(
      bool hasPreviousPass, bool hasNextPass
    ) = 0;

private:
    virtual void render(
      RenderPacket& packet, CommandBuffer& commandBuffer, u32 imageIndex
    ) = 0;

    Renderer& m_renderer;

protected:
    ResourceRef<Shader> m_shader;

private:
    Vec2<f32> m_viewportOffset;
    OwningPtr<RenderPassBackend> m_renderPassBackend;
    OwningPtr<Pipeline> m_pipeline;

public:
    const std::string name;
};

}  // namespace sl

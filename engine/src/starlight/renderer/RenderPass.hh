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

class RenderPassBase : public NonMovable, public Identificable<RenderPassBase> {
public:
    explicit RenderPassBase(
      Renderer& renderer, const Vec2<f32>& viewportOffset = { 0.0f, 0.0f },
      std::optional<std::string> name = {}
    );

    virtual void init(bool hasPreviousPass, bool hasNextPass) = 0;
    virtual void run(
      RenderPacket& packet, CommandBuffer& commandBuffer, u32 imageIndex
    ) = 0;

protected:
    virtual Rect2<u32> getViewport();

    RenderPassBackend::Properties createDefaultProperties(
      Attachment attachments, ClearFlags clearFlags = ClearFlags::none,
      RenderPassBackend::Type type = RenderPassBackend::Type::normal
    );

    virtual RenderPassBackend::Properties createProperties(
      bool hasPreviousPass, bool hasNextPass
    ) = 0;

    Renderer& m_renderer;
    OwningPtr<RenderPassBackend> m_renderPassBackend;
    Vec2<f32> m_viewportOffset;

public:
    const std::string name;
};

class RenderPass : public RenderPassBase {
public:
    explicit RenderPass(
      Renderer& renderer, ResourceRef<Shader> shader,
      const Vec2<f32>& viewportOffset = { 0.0f, 0.0f },
      std::optional<std::string> name = {}
    );

    void init(bool hasPreviousPass, bool hasNextPass);
    void run(RenderPacket& packet, CommandBuffer& commandBuffer, u32 imageIndex);

protected:
    void drawMesh(Mesh& mesh, CommandBuffer& buffer);

    ResourceRef<Shader> m_shader;
    OwningPtr<Pipeline> m_pipeline;

private:
    virtual void render(
      RenderPacket& packet, CommandBuffer& commandBuffer, u32 imageIndex
    ) = 0;
};

}  // namespace sl

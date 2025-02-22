#pragma once

#include <vector>
#include <optional>
#include <unordered_map>

#include "starlight/core/Core.hh"
#include "starlight/core/math/Core.hh"
#include "starlight/core/Enum.hh"
#include "starlight/core/Id.hh"
#include "starlight/core/Resource.hh"
#include "starlight/renderer/RenderPacket.hh"

#include "gpu/Texture.hh"
#include "gpu/CommandBuffer.hh"
#include "gpu/RenderPassBackend.hh"
#include "gpu/Shader.hh"

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
    virtual ~RenderPassBase() = default;

    virtual void init(bool hasPreviousPass, bool hasNextPass) = 0;
    virtual void run(
      RenderPacket& packet, CommandBuffer& commandBuffer, u32 imageIndex,
      u64 frameNumber
    ) = 0;

protected:
    virtual Rect2<u32> getViewport();

    RenderPassBackend::Properties generateRenderPassProperties(
      Attachment attachments, ClearFlags clearFlags = ClearFlags::none,
      RenderPassBackend::Type type = RenderPassBackend::Type::normal
    );

    virtual Pipeline::Properties createPipelineProperties();

    virtual RenderPassBackend::Properties createRenderPassProperties(
      bool hasPreviousPass, bool hasNextPass
    ) = 0;

    Renderer& m_renderer;
    UniquePointer<RenderPassBackend> m_renderPassBackend;
    Vec2<f32> m_viewportOffset;

public:
    const std::string name;
};

class RenderPass : public RenderPassBase {
public:
    explicit RenderPass(
      Renderer& renderer, SharedPointer<Shader> shader,
      const Vec2<f32>& viewportOffset = { 0.0f, 0.0f },
      std::optional<std::string> name = {}
    );

    void init(bool hasPreviousPass, bool hasNextPass);
    void run(
      RenderPacket& packet, CommandBuffer& commandBuffer, u32 imageIndex,
      u64 frameNumber
    );

private:
    SharedPointer<Shader> m_shader;
    UniquePointer<Pipeline> m_pipeline;
    UniquePointer<ShaderDataBinder> m_shaderDataBinder;

    std::unordered_map<u32, u32> m_localDescriptorSets;

    virtual void render(
      RenderPacket& packet, CommandBuffer& commandBuffer, u32 imageIndex,
      u64 frameNumber
    ) = 0;

protected:
    u32 getLocalDescriporSetId(u32 id);

    template <typename T>
    void setPushConstant(
      CommandBuffer& commandBuffer, const std::string& name, T&& value
    ) {
        m_shaderDataBinder->setPushConstant(
          *m_pipeline, commandBuffer, name, std::forward<T>(value)
        );
    }

    void setGlobalUniforms(
      CommandBuffer& commandBuffer, u64 frameNumber, u32 imageIndex,
      ShaderDataBinder::UniformCallback&& callback
    );

    void setLocalUniforms(
      CommandBuffer& commandBuffer, u64 frameNumber, u32 id, u32 imageIndex,
      ShaderDataBinder::UniformCallback&& callback
    );

    void drawMesh(Mesh& mesh, CommandBuffer& buffer);
};

}  // namespace sl

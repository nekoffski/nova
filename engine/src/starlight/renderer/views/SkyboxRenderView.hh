#pragma once

#include "starlight/core/memory/Memory.hh"

#include "RenderView.hh"

#include "starlight/renderer/gpu/RenderPass.hh"
#include "starlight/renderer/gpu/Shader.hh"
#include "starlight/renderer/Skybox.hh"

namespace sl {

class SkyboxRenderView : public RenderView {
public:
    explicit SkyboxRenderView(
      ResourceRef<Shader> shader = Skybox::getDefaultShader()
    );

    RenderPass::Properties getRenderPassProperties(
      RendererBackend& renderer, RenderPass::ChainFlags chainFlags
    ) const override;

    void init(RendererBackend& renderer, RenderPass& renderPass) override;
    void preRender(RendererBackend& renderer) override;

    void render(
      RendererBackend& renderer, const RenderPacket& packet,
      const RenderProperties& props, float deltaTime, CommandBuffer& commandBuffer,
      u8 imageIndex
    ) override;

    std::string_view getName() const override;

private:
    ResourceRef<Shader> m_shader;
};

}  // namespace sl
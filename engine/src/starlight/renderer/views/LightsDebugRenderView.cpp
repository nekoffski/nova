#include "LightsDebugRenderView.hh"

#include "starlight/core/window/Window.hh"

namespace sl {

static const std::string shaderName = "Builtin.Shader.LightsDebug";

LightsDebugRenderView::LightsDebugRenderView() :
    RenderView("LightsDebugRenderView", { 0.0f, 0.0f }),
    m_shader(Shader::load(shaderName)), m_mesh(Mesh::getUnitSphere()) {}

RenderPass::Properties LightsDebugRenderView::getRenderPassProperties(
  RendererBackend& renderer, [[maybe_unused]] RenderPass::ChainFlags chainFlags
) const {
    return getDefaultRenderPassProperties(
      renderer, Attachment::swapchainColor | Attachment::depth
    );
}

void LightsDebugRenderView::init(
  [[maybe_unused]] RendererBackend&, RenderPass& renderPass
) {
    m_shader->createPipeline(renderPass);
}

void LightsDebugRenderView::render(
  RendererBackend& renderer, const RenderPacket& packet,
  [[maybe_unused]] const RenderProperties& properties,
  [[maybe_unused]] float deltaTime, CommandBuffer& commandBuffer, u8 imageIndex
) {
    m_shader->use(commandBuffer);
    m_shader->setGlobalUniforms(commandBuffer, imageIndex, [&](auto& proxy) {
        proxy.set("view", packet.camera->getViewMatrix());
        proxy.set("projection", packet.camera->getProjectionMatrix());
    });

    for (auto& light : packet.pointLights) {
        const auto model =
          math::translate(identityMatrix, Vec3<f32>{ light.position });
        auto color = light.color;

        for (const auto& [distance, opacity] : light.getLODs()) {
            color.a = opacity;
            m_shader->setLocalUniforms(commandBuffer, [&](auto& proxy) {
                proxy.set("model", math::scale(model, Vec3<f32>{ distance }));
                proxy.set("color", color);
            });
            renderer.drawMesh(*m_mesh);
        }
    }
}

}  // namespace sl

#include "SkyboxRenderView.hh"

#include "starlight/core/window/Window.hh"

namespace sl {

SkyboxRenderView::SkyboxRenderView(
  const Vec2<f32>& viewportOffset, ResourceRef<Shader> shader
) : RenderView("SkyboxRenderView", viewportOffset), m_shader(shader) {}

RenderPass::Properties SkyboxRenderView::generateRenderPassProperties(
  RendererBackend& renderer, [[maybe_unused]] RenderPass::ChainFlags chainFlags
) {
    return generateDefaultRenderPassProperties(
      renderer, Attachment::swapchainColor, RenderPass::ClearFlags::color
    );
}

void SkyboxRenderView::init(
  [[maybe_unused]] RendererBackend&, RenderPass& renderPass
) {
    m_shader->createPipeline(renderPass);
}

void SkyboxRenderView::render(
  RendererBackend& renderer, RenderPacket& packet,
  [[maybe_unused]] const RenderProperties& properties,
  [[maybe_unused]] float deltaTime, CommandBuffer& commandBuffer, u8 imageIndex
) {
    auto skybox = packet.skybox;

    if (not skybox) {
        return;
    }

    auto camera = packet.camera;

    m_shader->use(commandBuffer);
    m_shader->setGlobalUniforms(commandBuffer, imageIndex, [&](auto& proxy) {
        auto viewMatrix  = camera->getViewMatrix();
        viewMatrix[3][0] = 0.0f;
        viewMatrix[3][1] = 0.0f;
        viewMatrix[3][2] = 0.0f;

        proxy.set("view", viewMatrix);
        proxy.set("projection", camera->getProjectionMatrix());
    });
    skybox->applyUniforms(m_shader, commandBuffer, imageIndex);

    renderer.drawMesh(*MeshFactory::get().getCube());
}

}  // namespace sl

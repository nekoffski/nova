#include "SkyboxRenderPass.hh"

#include "starlight/renderer/factories/ShaderFactory.hh"
#include "starlight/renderer/factories/SkyboxFactory.hh"

namespace sl {

SkyboxRenderPass::SkyboxRenderPass(Renderer& renderer
) : RenderPass(renderer, SkyboxFactory::get().getDefaultShader()) {}

RenderPassBackend::Properties SkyboxRenderPass::createProperties(
  bool hasPreviousPass, bool hasNextPass
) {
    return createDefaultProperties(Attachment::swapchainColor, ClearFlags::color);
}

void SkyboxRenderPass::render(
  RenderPacket& packet, CommandBuffer& commandBuffer, u32 imageIndex
) {
    auto skybox = packet.skybox;

    if (not skybox) return;

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

    //     renderer.drawMesh(*MeshFactory::get().getCube());
}

}  // namespace sl

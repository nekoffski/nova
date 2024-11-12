#include "SkyboxRenderView.hh"

#include "starlight/core/window/Window.hh"

namespace sl {

SkyboxRenderView::SkyboxRenderView(Skybox* skybox) : m_skybox(skybox) {}

RenderPass::Properties SkyboxRenderView::getRenderPassProperties(
  RendererBackend& renderer, [[maybe_unused]] RenderPass::ChainFlags chainFlags
) const {
    const auto backgroundColor = (1.0f / 255.0f) * Vec4<f32>{ 11, 16, 47, 255 };

    auto props       = getDefaultRenderPassProperties();
    props.clearColor = backgroundColor;
    props.clearFlags = RenderPass::ClearFlags::color;

    const auto swapchainImageCount = renderer.getSwapchainImageCount();
    props.renderTargets.reserve(swapchainImageCount);

    RenderTarget renderTarget;
    renderTarget.size = props.rect.size;

    for (u8 i = 0; i < swapchainImageCount; ++i) {
        renderTarget.attachments = { renderer.getSwapchainTexture(i) };
        props.renderTargets.push_back(renderTarget);
    }

    props.includeDepthAttachment = false;
    return props;
}

void SkyboxRenderView::init(RenderPass& renderPass) {
    m_skybox->getShader()->createPipeline(renderPass);
}

void SkyboxRenderView::preRender(RendererBackend& renderer) {
    Rect2<u32> viewport{
        .offset = Vec2<u32>{ 0, 0 },
        .size   = Window::get().getFramebufferSize(),
    };
    renderer.setViewport(viewport);
}

void SkyboxRenderView::render(
  RendererBackend& renderer, const RenderPacket& packet,
  [[maybe_unused]] const RenderProperties& properties,
  [[maybe_unused]] float deltaTime, CommandBuffer& commandBuffer, u8 imageIndex
) {
    auto shader = m_skybox->getShader();
    auto camera = packet.camera;

    shader->use(commandBuffer);

    shader->setGlobalUniforms(commandBuffer, imageIndex, [&](auto& proxy) {
        auto viewMatrix  = camera->getViewMatrix();
        viewMatrix[3][0] = 0.0f;
        viewMatrix[3][1] = 0.0f;
        viewMatrix[3][2] = 0.0f;

        proxy.set("view", viewMatrix);
        proxy.set("projection", camera->getProjectionMatrix());
    });

    shader->setInstanceUniforms(
      commandBuffer, m_skybox->getInstanceId(), imageIndex,
      [&](auto& proxy) { proxy.set("cubeTexture", m_skybox->getCubeMap()); }
    );

    renderer.drawMesh(*m_skybox->getMesh());
}

}  // namespace sl

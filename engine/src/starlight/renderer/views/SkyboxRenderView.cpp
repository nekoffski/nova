#include "SkyboxRenderView.hh"

#include "starlight/core/window/Window.hh"

namespace sl {

SkyboxRenderView::SkyboxRenderView(ResourceRef<Shader> shader) : m_shader(shader) {}

RenderPass::Properties SkyboxRenderView::getRenderPassProperties(
  RendererBackend& renderer, [[maybe_unused]] RenderPass::ChainFlags chainFlags
) const {
    auto props       = getDefaultRenderPassProperties();
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

void SkyboxRenderView::init(
  [[maybe_unused]] RendererBackend&, RenderPass& renderPass
) {
    m_shader->createPipeline(renderPass);
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
    auto skybox = packet.skybox;

    if (not skybox) {
        LOG_WARN("Skybox == null, skipping SkyboxRenderView");
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

    renderer.drawMesh(*Mesh::getCube());
}

std::string_view SkyboxRenderView::getName() const { return "SkyboxRenderView"; }

}  // namespace sl

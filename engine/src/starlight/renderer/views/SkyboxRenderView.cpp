#include "SkyboxRenderView.hh"

#include "starlight/core/window/Window.hh"

namespace sl {

SkyboxRenderView::SkyboxRenderView(Skybox* skybox) : m_skybox(skybox) {}

void SkyboxRenderView::init(
  RendererBackend& renderer, const InitProperties& initProperties
) {
    LOG_TRACE("Initializing SkyboxRenderView");
    auto backgroundColor = (1.0f / 255.0f) * glm::vec4{ 11, 16, 47, 255 };

    RenderPass::Properties renderPassProperties{
        .rect       = Rect2u32{ Vec2<u32>{ 0u, 0u }, initProperties.viewportSize },
        .clearColor = backgroundColor,
        .clearFlags = RenderPass::clearColorBuffer,
        .hasPreviousPass = initProperties.hasPreviousView,
        .hasNextPass     = initProperties.hasNextView,
        .renderTargets   = {}
    };

    RenderTarget renderTarget{
        .size = initProperties.viewportSize, .attachments = {}
    };

    for (u8 i = 0; i < 3; ++i) {
        renderTarget.attachments = { renderer.getSwapchainTexture(i) };
        renderPassProperties.renderTargets.push_back(renderTarget);
    }

    // TODO: it doesn't look right, shouldn't we just overload some kind of method to
    // return render pass properties, and then engine could use this to create it
    // implicitly

    m_renderPass = RenderPass::create(renderer, renderPassProperties);
    m_skybox->getShader()->createPipeline(*m_renderPass);
    LOG_TRACE("SkyboxRenderView initialized");
}

void SkyboxRenderView::render(
  RendererBackend& renderer, [[maybe_unused]] const RenderPacket& packet,
  [[maybe_unused]] const RenderProperties& properties,
  [[maybe_unused]] float deltaTime
) {
    Rect2<u32> viewport{
        .offset = Vec2<u32>{ 0, 0 },
        .size   = Window::get().getFramebufferSize(),
    };
    renderer.setViewport(viewport);

    m_renderPass->run(
      renderer.getCommandBuffer(), renderer.getImageIndex(),
      [&](CommandBuffer& commandBuffer, u8 imageIndex) {
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
    );
}

void SkyboxRenderView::onViewportResize(
  RendererBackend& renderer, Vec2<u32> viewportSize
) {
    // TODO: make it generic
    std::vector<RenderTarget> renderTargets;
    renderTargets.reserve(3);

    RenderTarget renderTarget{ .size = viewportSize, .attachments = {} };

    for (u8 i = 0; i < 3; ++i) {
        renderTarget.attachments = { renderer.getSwapchainTexture(i) };
        renderTargets.push_back(renderTarget);
    }
    m_renderPass->regenerateRenderTargets(renderTargets);
    m_renderPass->setRectSize(viewportSize);
}

}  // namespace sl

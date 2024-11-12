#include "LightsDebugRenderView.hh"

#include "starlight/core/window/Window.hh"

namespace sl {

static const std::string shaderName = "Builtin.Shader.LightsDebug";

LightsDebugRenderView::LightsDebugRenderView() :
    m_shader(Shader::load(shaderName)),
    // m_mesh(Mesh::getCube())

    // change the interface! SphereProperties
    m_mesh(Mesh::load(SphereProperties{ 25, 25, 0.25f })) {}

RenderPass::Properties LightsDebugRenderView::getRenderPassProperties(
  RendererBackend& renderer, [[maybe_unused]] RenderPass::ChainFlags chainFlags
) const {
    auto props = getDefaultRenderPassProperties();

    props.clearFlags               = RenderPass::ClearFlags::none;
    const auto swapchainImageCount = renderer.getSwapchainImageCount();
    props.renderTargets.reserve(swapchainImageCount);

    RenderTarget renderTarget;
    renderTarget.size = props.rect.size;

    for (u8 i = 0; i < swapchainImageCount; ++i) {
        renderTarget.attachments = {
            renderer.getSwapchainTexture(i), renderer.getDepthTexture()
        };
        props.renderTargets.push_back(renderTarget);
    }

    props.includeDepthAttachment = true;
    return props;
}

void LightsDebugRenderView::init(RenderPass& renderPass) {
    m_shader->createPipeline(renderPass);
}

void LightsDebugRenderView::preRender(RendererBackend& renderer) {
    Rect2<u32> viewport{
        .offset = Vec2<u32>{ 0, 0 },
        .size   = Window::get().getFramebufferSize(),
    };
    renderer.setViewport(viewport);
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
        m_shader->setLocalUniforms(commandBuffer, [&](auto& proxy) {
            proxy.set(
              "model", math::translate(identityMatrix, Vec3<f32>{ light.position })
            );
            proxy.set("color", light.color);
        });
        renderer.drawMesh(*m_mesh);
    }
}

}  // namespace sl

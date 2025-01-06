#include "ShadowMapsRenderView.hh"

#include "starlight/core/window/Window.hh"
#include "starlight/core/Algorithms.hh"

namespace sl {

ShadowMapsRenderView::ShadowMapsRenderView(const Vec2<f32>& viewportOffset) :
    RenderView("ShadowMapsRenderView", viewportOffset),
    m_shader(ShaderManager::get().load("Builtin.Shader.ShadowMaps")) {}

RenderPass::Properties ShadowMapsRenderView::generateRenderPassProperties(
  RendererBackend& renderer, [[maybe_unused]] RenderPass::ChainFlags chainFlags
) {
    auto imageData   = Texture::ImageData::createDefault();
    imageData.width  = 1024;
    imageData.height = 1024;
    imageData.flags  = Texture::Flags::writable;

    const auto swapchainImageCount = renderer.getSwapchainImageCount();

    for (u32 i = 0; i < swapchainImageCount; ++i)
        m_shadowMaps.push_back(Texture::create(renderer, imageData));

    RenderPass::Properties props;

    props.clearColor = Vec4<f32>{ 0.0f };
    props.clearFlags = RenderPass::ClearFlags::color;
    props.rect       = Rect2<u32>{
        Vec2<u32>{ 0u,    0u    },
        Vec2<u32>{ 1024u, 1024u }
    };

    props.renderTargets.reserve(swapchainImageCount);

    RenderTarget renderTarget;
    renderTarget.size = props.rect.size;

    for (u8 i = 0; i < swapchainImageCount; ++i) {
        renderTarget.depthAttachment = m_shadowMaps[i].get();
        props.renderTargets.push_back(renderTarget);
    }

    return props;
}

void ShadowMapsRenderView::init(
  [[maybe_unused]] RendererBackend& backend, RenderPass& renderPass
) {
    m_shader->createPipeline(renderPass);
}

void ShadowMapsRenderView::render(
  RendererBackend& renderer, RenderPacket& packet,
  [[maybe_unused]] const RenderProperties& properties,
  [[maybe_unused]] float deltaTime, CommandBuffer& commandBuffer, u8 imageIndex
) {
    auto camera               = packet.camera;
    const auto cameraPosition = camera->getPosition();

    m_shader->use(commandBuffer);
    m_shader->setGlobalUniforms(
      commandBuffer, imageIndex,
      [&](Shader::UniformProxy& proxy) {
          proxy.set("view", camera->getViewMatrix());
          proxy.set("projection", camera->getProjectionMatrix());
          proxy.set("viewPosition", cameraPosition);
      }
    );

    for (auto& [worldTransform, mesh, _] : packet.entities) {
        m_shader->setLocalUniforms(commandBuffer, [&](Shader::UniformProxy& proxy) {
            proxy.set("model", worldTransform);
        });
        renderer.drawMesh(*mesh);
    }
    packet.shadowMaps.push_back(m_shadowMaps[imageIndex].get());
}

Rect2<u32> ShadowMapsRenderView::getViewport() const {
    return Rect2<u32>{
        Vec2<u32>{ 0u,   0u    },
        Vec2<u32>{ 1024, 1024u }
    };
}

}  // namespace sl

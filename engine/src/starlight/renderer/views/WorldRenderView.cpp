#include "WorldRenderView.hh"

#include "starlight/core/window/Window.hh"

namespace sl {

WorldRenderView::WorldRenderView(Shader* shader) : m_shader(shader) {}

RenderPass::Properties WorldRenderView::getRenderPassProperties(
  RendererBackend& renderer, RenderPass::ChainFlags chainFlags
) const {
    auto props = getDefaultRenderPassProperties();

    auto clearFlags =
      RenderPass::ClearFlags::depth | RenderPass::ClearFlags::stencil;
    if (not isFlagEnabled(chainFlags, RenderPass::ChainFlags::hasPrevious))
        clearFlags |= RenderPass::ClearFlags::color;

    props.clearFlags = clearFlags;

    RenderTarget renderTarget;
    renderTarget.size = props.rect.size;

    const auto swapchainImageCount = renderer.getSwapchainImageCount();
    props.renderTargets.reserve(swapchainImageCount);

    for (u8 i = 0; i < swapchainImageCount; ++i) {
        renderTarget.attachments = {
            renderer.getSwapchainTexture(i), renderer.getDepthTexture()
        };
        props.renderTargets.push_back(renderTarget);
    }

    return props;
}

void WorldRenderView::init(RenderPass& renderPass) {
    m_shader->createPipeline(renderPass);
}

void WorldRenderView::preRender(RendererBackend& renderer) {
    Rect2<u32> viewport{
        .offset = Vec2<u32>{ 0, 0 },
        .size   = Window::get().getFramebufferSize(),
    };
    renderer.setViewport(viewport);
}

struct MeshRenderData {
    Mesh* mesh;
    Material* material;
    Mat4<f32> modelMatrix;
    float cameraDistance;
};

void WorldRenderView::render(
  RendererBackend& renderer, const RenderPacket& packet,
  const RenderProperties& properties, [[maybe_unused]] float deltaTime,
  CommandBuffer& commandBuffer, u8 imageIndex
) {
    Vec4<f32> ambientColor(0.3f, 0.3f, 0.3f, 1.0f);
    auto camera               = packet.camera;
    const auto cameraPosition = camera->getPosition();

    m_shader->use(commandBuffer);
    m_shader->setGlobalUniforms(
      commandBuffer, imageIndex,
      [&](Shader::UniformProxy& proxy) {
          proxy.set("view", camera->getViewMatrix());
          proxy.set("projection", camera->getProjectionMatrix());
          proxy.set("viewPosition", cameraPosition);
          proxy.set("ambientColor", ambientColor);
          proxy.set("renderMode", static_cast<int>(properties.renderMode));

          const auto pointLightCount = packet.pointLights.size();
          proxy.set("pointLightCount", pointLightCount);

          if (pointLightCount > 0)
              proxy.set("pointLights", packet.pointLights.data());
      }
    );

    std::vector<MeshRenderData> meshes;
    std::vector<MeshRenderData> transparentGeometries;
    meshes.reserve(256);
    transparentGeometries.reserve(128);

    for (auto& [worldTransform, mesh, material] : packet.entities) {
        if (material->isTransparent()) {
            auto center         = worldTransform * mesh->getExtent().center;
            auto cameraDistance = glm::distance2(cameraPosition, center);
            transparentGeometries.emplace_back(
              mesh, material, worldTransform, cameraDistance
            );
        } else {
            meshes.emplace_back(mesh, material, worldTransform);
        }
    }

    std::sort(
      transparentGeometries.begin(), transparentGeometries.end(),
      [](auto& lhs, auto& rhs) -> bool {
          return lhs.cameraDistance < rhs.cameraDistance;
      }
    );
    std::move(
      transparentGeometries.begin(), transparentGeometries.end(),
      std::back_inserter(meshes)
    );
    transparentGeometries.clear();

    for (auto& [mesh, material, model, _] : meshes) {
        m_shader->setLocalUniforms(commandBuffer, [&](Shader::UniformProxy& proxy) {
            proxy.set("model", model);
        });

        material->applyUniforms(
          *m_shader, commandBuffer, imageIndex, properties.frameNumber
        );
        renderer.drawMesh(*mesh);
    }
}

// void WorldRenderView::onViewportResize(
//   RendererBackend& renderer, Vec2<u32> viewportSize
// ) {
//     // TODO: get swapchain images count from backend
//     std::vector<RenderTarget> renderTargets;
//     renderTargets.reserve(3);

//     RenderTarget renderTarget{ .size = viewportSize, .attachments = {} };

//     for (u8 i = 0; i < 3; ++i) {
//         renderTarget.attachments = {
//             renderer.getSwapchainTexture(i), renderer.getDepthTexture()
//         };
//         renderTargets.push_back(renderTarget);
//     }
//     m_renderPass->regenerateRenderTargets(renderTargets);
//     m_renderPass->setRectSize(viewportSize);
// }

}  // namespace sl
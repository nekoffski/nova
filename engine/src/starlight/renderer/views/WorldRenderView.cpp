#include "WorldRenderView.hh"

namespace sl {

WorldRenderView::WorldRenderView(Shader* shader) : m_shader(shader) {}

void WorldRenderView::init(
  RendererBackend& renderer, const InitProperties& initProperties
) {
    LOG_TRACE("Initializing WorldRenderView");
    auto backgroundColor = (1.0f / 255.0f) * glm::vec4{ 11, 16, 47, 255 };

    u8 clearFlags = RenderPass::clearDepthBuffer | RenderPass::clearStencilBuffer;
    if (not initProperties.hasPreviousView)
        clearFlags |= RenderPass::clearColorBuffer;

    RenderPass::Properties renderPassProperties{
        .rect       = Rect2u32{Vec2<u32>{ 0u, 0u }, initProperties.viewportSize},
        .clearColor = backgroundColor,
        .clearFlags = clearFlags,
        .hasPreviousPass = initProperties.hasPreviousView,
        .hasNextPass     = initProperties.hasNextView
    };

    RenderTarget renderTarget{ .size = initProperties.viewportSize };

    for (u8 i = 0; i < 3; ++i) {
        renderTarget.attachments = {
            renderer.getSwapchainTexture(i), renderer.getDepthTexture()
        };
        renderPassProperties.renderTargets.push_back(renderTarget);
    }

    m_renderPass = RenderPass::create(renderer, renderPassProperties);
    m_shader->createPipeline(*m_renderPass);
    LOG_TRACE("WorldRenderView initialized");
}

struct MeshRenderData {
    Mesh* mesh;
    Material* material;
    Mat4<f32> modelMatrix;
    float cameraDistance;
};

void WorldRenderView::render(
  RendererBackend& renderer, const RenderPacket& packet,
  const RenderProperties& properties, [[maybe_unused]] float deltaTime
) {
    renderer.setViewport(packet.viewport);
    m_renderPass->run(
      renderer.getCommandBuffer(), renderer.getImageIndex(),
      [&](CommandBuffer& commandBuffer, u32 imageIndex) {
          glm::vec4 ambientColor(0.3f, 0.3f, 0.3f, 1.0f);

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
              m_shader->setLocalUniforms(
                commandBuffer,
                [&](Shader::UniformProxy& proxy) { proxy.set("model", model); }
              );

              material->applyUniforms(
                *m_shader, commandBuffer, imageIndex, properties.frameNumber
              );
              renderer.drawMesh(*mesh);
          }
      }
    );
}

void WorldRenderView::onViewportResize(
  RendererBackend& renderer, Vec2<u32> viewportSize
) {
    // TODO: get swapchain images count from backend
    std::vector<RenderTarget> renderTargets;
    renderTargets.reserve(3);

    RenderTarget renderTarget{ .size = viewportSize };

    for (u8 i = 0; i < 3; ++i) {
        renderTarget.attachments = {
            renderer.getSwapchainTexture(i), renderer.getDepthTexture()
        };
        renderTargets.push_back(renderTarget);
    }
    m_renderPass->regenerateRenderTargets(renderTargets);
    m_renderPass->setRectSize(viewportSize);
}

}  // namespace sl
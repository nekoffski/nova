#include "WorldRenderView.hh"

#include "starlight/core/window/Window.hh"
#include "starlight/core/Algorithms.hh"

namespace sl {

WorldRenderView::WorldRenderView(
  const Vec2<f32>& viewportOffset, ResourceRef<Shader> shader
) : RenderView("WorldRenderView", viewportOffset), m_shader(shader) {}

RenderPass::Properties WorldRenderView::generateRenderPassProperties(
  RendererBackend& renderer, RenderPass::ChainFlags chainFlags
) {
    auto clearFlags =
      RenderPass::ClearFlags::depth | RenderPass::ClearFlags::stencil;
    if (not isFlagEnabled(chainFlags, RenderPass::ChainFlags::hasPrevious))
        clearFlags |= RenderPass::ClearFlags::color;
    return generateDefaultRenderPassProperties(
      renderer, Attachment::swapchainColor | Attachment::depth, clearFlags
    );
}

void WorldRenderView::init(
  [[maybe_unused]] RendererBackend&, RenderPass& renderPass
) {
    m_shader->createPipeline(renderPass);
}

struct MeshRenderData {
    Mesh* mesh;
    Material* material;
    Mat4<f32> modelMatrix;
    float cameraDistance;
};

void WorldRenderView::render(
  RendererBackend& renderer, RenderPacket& packet,
  const RenderProperties& properties, [[maybe_unused]] float deltaTime,
  CommandBuffer& commandBuffer, u8 imageIndex
) {
    Vec4<f32> ambientColor(0.05f, 0.05f, 0.05f, 1.0f);
    auto camera               = packet.camera;
    const auto cameraPosition = camera->getPosition();

    m_shader->use(commandBuffer);
    m_shader->setGlobalUniforms(
      commandBuffer, imageIndex,
      [&](Shader::UniformProxy& proxy) {
          auto renderMode = static_cast<int>(properties.renderMode);

          Mat4<f32> biasMatrix(
            0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f,
            0.5f, 0.5f, 0.5f, 1.0f
          );

          auto depthMVP =
            math::ortho<float>(-10.0f, 10.0f, -10.0f, 10.0f, -10.0f, 20)
            * math::lookAt(
              -packet.directionalLights[0].direction, Vec3<f32>(0.0f),
              Vec3<f32>(0.0f, 1.0f, 0.0f)
            );

          proxy.set("view", camera->getViewMatrix());
          proxy.set("projection", camera->getProjectionMatrix());
          proxy.set("depthMVP", biasMatrix * depthMVP);
          proxy.set("viewPosition", cameraPosition);
          proxy.set("ambientColor", ambientColor);
          proxy.set("renderMode", &renderMode);
          proxy.set("shadowMap", packet.shadowMaps[0]);

          const auto pointLightCount = packet.pointLights.size();

          if (pointLightCount > 0) {
              const auto shaderBulk = transform<PointLight::ShaderData>(
                packet.pointLights,
                [](const auto& light) { return light.getShaderData(); }
              );
              proxy.set("pointLights", shaderBulk);
          }

          const auto directionalLightCount = packet.directionalLights.size();

          if (directionalLightCount > 0)
              proxy.set("directionalLights", packet.directionalLights);

          proxy.set("pointLightCount", &pointLightCount);
          proxy.set("directionalLightCount", &directionalLightCount);
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
          m_shader, commandBuffer, imageIndex, properties.frameNumber
        );
        renderer.drawMesh(*mesh);
    }
}

}  // namespace sl

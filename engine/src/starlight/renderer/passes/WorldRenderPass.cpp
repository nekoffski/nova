#include "WorldRenderPass.hh"

#include "starlight/core/Algorithms.hh"

#include "starlight/renderer/factories/ShaderFactory.hh"
#include "starlight/renderer/factories/MeshFactory.hh"

#include "starlight/renderer/RenderMode.hh"

namespace sl {

WorldRenderPass::WorldRenderPass(Renderer& renderer
) : RenderPass(renderer, ShaderFactory::get().load("Builtin.Shader.Material")) {}

RenderPassBackend::Properties WorldRenderPass::createProperties(
  [[maybe_unused]] bool hasPreviousPass, [[maybe_unused]] bool hasNextPass
) {
    return createDefaultProperties(
      Attachment::swapchainColor | Attachment::depth, ClearFlags::depth
    );
}

struct MeshRenderData {
    Mesh* mesh;
    Material* material;
    Mat4<f32> modelMatrix;
    float cameraDistance;
};

void WorldRenderPass::render(
  RenderPacket& packet, CommandBuffer& commandBuffer, u32 imageIndex
) {
    Vec4<f32> ambientColor(0.05f, 0.05f, 0.05f, 1.0f);
    auto camera               = packet.camera;
    const auto cameraPosition = camera->getPosition();

    m_shader->use(commandBuffer);
    m_shader->setGlobalUniforms(
      commandBuffer, imageIndex,
      [&](Shader::UniformProxy& proxy) {
          auto depthMVP =
            math::ortho<float>(-5.0f, 5.0f, -5.0f, 5.0f, -5.0f, 20.0f)
            * math::lookAt(
              -packet.directionalLights[0].direction, Vec3<f32>(0.0f, 0.0f, 0.0f),
              Vec3<f32>(0.0f, 1.0f, 0.0f)
            );

          proxy.set("view", camera->getViewMatrix());
          proxy.set("projection", camera->getProjectionMatrix());
          proxy.set("depthMVP", depthMVP);
          proxy.set("viewPosition", cameraPosition);
          proxy.set("ambientColor", ambientColor);
          proxy.set("renderMode", static_cast<int>(RenderMode::standard));
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
        material->applyUniforms(m_shader, commandBuffer, imageIndex, 1);
        m_shader->setLocalUniforms(commandBuffer, [&](Shader::UniformProxy& proxy) {
            proxy.set("model", model);
        });

        drawMesh(*mesh, commandBuffer);
    }
}

}  // namespace sl

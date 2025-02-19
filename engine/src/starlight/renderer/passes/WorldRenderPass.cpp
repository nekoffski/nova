#include "WorldRenderPass.hh"

#include "starlight/core/Utils.hh"

#include "starlight/renderer/factories/ShaderFactory.hh"
#include "starlight/renderer/factories/MeshFactory.hh"
#include "starlight/renderer/Core.hh"

namespace sl {

WorldRenderPass::WorldRenderPass(
  Renderer& renderer, const Vec2<f32>& viewportOffset
) :
    RenderPass(
      renderer, ShaderFactory::get().load("Builtin.Shader.Material"), viewportOffset,
      "WorldRenderPass"
    ) {}

RenderPassBackend::Properties WorldRenderPass::createRenderPassProperties(
  [[maybe_unused]] bool hasPreviousPass, [[maybe_unused]] bool hasNextPass
) {
    return generateRenderPassProperties(
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
  RenderPacket& packet, CommandBuffer& commandBuffer, u32 imageIndex,
  [[maybe_unused]] u64 frameNumber
) {
    Vec4<f32> ambientColor(0.05f, 0.05f, 0.05f, 1.0f);
    auto camera               = packet.camera;
    const auto cameraPosition = camera->getPosition();

    setGlobalUniforms(commandBuffer, imageIndex, [&](auto& setter) {
        auto depthMVP =
          math::ortho<float>(-5.0f, 5.0f, -5.0f, 5.0f, -5.0f, 20.0f)
          * math::lookAt(
            -packet.directionalLights[0].direction, Vec3<f32>(0.0f, 0.0f, 0.0f),
            Vec3<f32>(0.0f, 1.0f, 0.0f)
          );

        setter.set("view", camera->getViewMatrix());
        setter.set("projection", camera->getProjectionMatrix());
        setter.set("depthMVP", depthMVP);
        setter.set("viewPosition", cameraPosition);
        setter.set("ambientColor", ambientColor);
        setter.set("mode", static_cast<int>(RenderMode::standard));
        setter.set("shadowMap", packet.shadowMaps[0]);

        const auto pointLightCount = packet.pointLights.size();

        if (pointLightCount > 0) {
            const auto shaderBulk = transform<PointLight::ShaderData>(
              packet.pointLights,
              [](const auto& light) { return light.getShaderData(); }
            );
            setter.set("pointLights", shaderBulk);
        }

        const auto directionalLightCount = packet.directionalLights.size();

        if (directionalLightCount > 0)
            setter.set("directionalLights", packet.directionalLights);

        setter.set("pointLightCount", &pointLightCount);
        setter.set("directionalLightCount", &directionalLightCount);
    });

    std::vector<MeshRenderData> meshes;
    std::vector<MeshRenderData> transparentGeometries;
    meshes.reserve(256);
    transparentGeometries.reserve(128);

    for (auto& [worldTransform, mesh, material] : packet.entities) {
        if (material->isTransparent()) {
            auto center         = worldTransform * mesh->getExtent().center;
            auto cameraDistance = glm::distance2(cameraPosition, center);
            transparentGeometries
              .emplace_back(mesh, material, worldTransform, cameraDistance);
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
        setLocalUniforms(
          commandBuffer, getLocalDescriporSetId(material->getId()), imageIndex,
          [&](auto& setter) {
              setter.set("diffuseColor", material->diffuseColor);
              setter.set("shininess", material->shininess);
              setter.set("diffuseMap", material->textures.diffuse.get());
              setter.set("specularMap", material->textures.specular.get());
              setter.set("normalMap", material->textures.normal.get());
          }
        );
        setPushConstant(commandBuffer, "model", model);
        drawMesh(*mesh, commandBuffer);
    }
}

}  // namespace sl

#include "Scene.hh"

#include "starlight/renderer/MeshComposite.hh"
#include "starlight/renderer/light/PointLight.hh"

namespace sl {

static constexpr u32 maxPointLights       = 5;
static constexpr u32 maxDirectionalLights = 5;

Scene::Scene(Camera* camera
) : camera(camera), skybox(nullptr), m_entities(maxEntities) {}

RenderPacket Scene::getRenderPacket() {
    RenderPacket packet{};
    packet.camera = camera;

    packet.directionalLights.reserve(maxDirectionalLights);
    packet.pointLights.reserve(maxPointLights);

    m_componentManager.getComponentContainer<MeshComposite>().forEach(
      [&](Component<MeshComposite>& meshComposite) {
          meshComposite.data().traverse([&](MeshComposite::Node& node) {
              for (auto& instance : node.getInstances()) {
                  packet.entities.emplace_back(
                    instance.getWorld(), node.mesh.get(), node.material.get()
                  );
              }
          });
      }
    );

    m_componentManager.getComponentContainer<PointLight>().forEach(
      [&](Component<PointLight>& light) {
          packet.pointLights.push_back(light.data());
      }
    );

    m_componentManager.getComponentContainer<DirectionalLight>().forEach(
      [&](Component<DirectionalLight>& light) {
          packet.directionalLights.push_back(light.data());
      }
    );

    packet.directionalLights.emplace_back();

    // mock for testing
    // PointLight light;

    // light.data.position.x = 1.0f;
    // light.data.color      = Vec4<f32>{ 1.0f, 0.5f, 0.3f, 1.0f };
    // packet.pointLights.push_back(light);

    // light.data.position.x = 0.0f;
    // light.data.position.y = 1.0f;
    // light.data.position.z = 1.0f;
    // light.data.color      = Vec4<f32>{ 0.5f, 0.5f, 0.1f, 1.0f };
    // packet.pointLights.push_back(light);

    packet.skybox = skybox.get();

    return packet;
}

void Scene::clear() {
    skybox = nullptr;
    m_entities.clear();
}

Entity& Scene::addEntity(std::optional<std::string> name) {
    auto record = m_entities.emplace(m_componentManager, name);
    log::expect(record, "Could not add entity");
    return *record;
}

}  // namespace sl

#include "Scene.hh"

#include "starlight/renderer/MeshComposite.hh"

namespace sl {

Scene::Scene(Window& window, Camera* camera) : m_window(window), m_camera(camera) {}

RenderPacket Scene::getRenderPacket() {
    RenderPacket packet{};
    packet.camera = m_camera;

    m_componentManager.getComponentContainer<MeshComposite>().forEach(
      [&](Component<MeshComposite>& meshComposite) {
          meshComposite.data().traverse([&](MeshComposite::Node& node) {
              for (auto& instance : node.getInstances()) {
                  packet.entities.emplace_back(
                    instance.getWorld(), node.getMesh(), node.getMaterial()
                  );
              }
          });
      }
    );

    // mock for testing
    PointLight light;

    light.position.x = 1.0f;
    light.color      = Vec4<f32>{ 1.0f, 0.5f, 0.3f, 1.0f };
    packet.pointLights.push_back(light);

    // light.position.y = 1.0f;
    // light.position.z = 1.0f;
    // light.position.x = 0.0f;
    // light.color      = Vec4<f32>{ 0.5f, 0.5f, 0.1f, 1.0f };
    // packet.pointLights.push_back(light);

    return packet;
}

void Scene::setCamera(Camera* camera) { m_camera = camera; }

Entity& Scene::addEntity(std::optional<std::string> name) {
    m_entities.emplace_back(m_componentManager, name);
    return m_entities.back();
}

}  // namespace sl

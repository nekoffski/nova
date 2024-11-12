#include "EntityInspectorPanel.hh"

#include <ImGuizmo.h>

EntityInspectorPanel::EntityInspectorPanel(
  sl::Scene* scene, UIState* state, Logger* logger
) :
    m_scene(scene), m_state(state), m_logger(logger),
    m_translationSlider("Translation", { .min = -15.0, .max = 15.0f, .step = 0.1f }),
    m_scaleSlider(
      "Scale", { .min = 0.0, .max = 5.0f, .step = 0.1f }, sl::Vec3<sl::f32>{ 1.0f }
    ),
    m_orientationSlider("Orientation", { .min = -3.14, .max = 3.14, .step = 0.01f }),
    m_selectedComponentIndex(0) {}

void EntityInspectorPanel::render() {
    if (m_state->selectedEntityId)
        renderEntityUI(*m_state->selectedEntityId);
    else
        sl::ui::text("No entity selected");
}

void EntityInspectorPanel::renderEntityUI(sl::u64 entityId) {
    // auto entity = m_scene->getEntity(entityId);
    // auto& data  = m_entitiesData[entity->getId()];

    // sl::ui::namedScope(fmt::format("Entity_{}", entity->getId()), [&]() {
    //     sl::ui::text("Entity: {}/{}", entity->getId(), entity->getName());
    //     sl::ui::separator();

    //     std::vector<const char*> components = {
    //         "Mesh", "Transform", "Material", "Point light"
    //     };

    //     if (sl::ui::button("Add")) {
    //         // if (m_selectedComponentIndex == 0) {
    //         //     entity->addComponent<sl::MeshComponent>();
    //         // } else if (m_selectedComponentIndex == 1) {
    //         //     entity->addComponent<sl::TransformComponent>();
    //         // } else if (m_selectedComponentIndex == 2) {
    //         //     entity->addComponent<sl::MaterialComponent>();
    //         // } else if (m_selectedComponentIndex == 3) {
    //         //     entity->addComponent<sl::PointLightComponent>();
    //         // }
    //     }

    //     sl::ui::sameLine();

    //     ImGui::Combo(
    //       "##combo2", &m_selectedComponentIndex, components.data(),
    //       components.size()
    //     );
    //     sl::ui::separator();

    //     if (entity->hasComponent<sl::MeshComponent>()) {
    //         sl::ui::treeNode(
    //           ICON_FA_PROJECT_DIAGRAM "  Mesh",
    //           [&]([[maybe_unused]] bool) {
    //               auto meshComponent = entity->getComponent<sl::MeshComponent>();
    //               auto meshesNames   = sl::MeshManager::get().getNames();

    //               std::vector<const char*> names = { "None" };
    //               for (auto& name : meshesNames) names.push_back(name.c_str());

    //               if (ImGui::Combo(
    //                     "##combo", &data.selectedMeshIndex, names.data(),
    //                     names.size()
    //                   )) {
    //                   if (data.selectedMeshIndex != 0) {
    //                       const auto meshName = names[data.selectedMeshIndex];
    //                       meshComponent->mesh =
    //                         sl::MeshManager::get().acquire(meshName);
    //                   } else {
    //                       meshComponent->mesh = nullptr;
    //                   }
    //               }
    //           }
    //         );
    //         sl::ui::separator();
    //     }

    //     if (entity->hasComponent<sl::MaterialComponent>()) {
    //         sl::ui::treeNode(
    //           ICON_FA_GLOBE_AMERICAS "  Material",
    //           [&]([[maybe_unused]] bool) {
    //               auto materialComponent =
    //                 entity->getComponent<sl::MaterialComponent>();
    //               auto materialNames = sl::MaterialManager::get().getNames();

    //               std::vector<const char*> names = { "None" };
    //               for (auto& name : materialNames) names.push_back(name.c_str());

    //               if (ImGui::Combo(
    //                     "##combo333", &data.selectedMaterialIndex, names.data(),
    //                     names.size()
    //                   )) {
    //                   if (data.selectedMaterialIndex != 0) {
    //                       const auto materialName =
    //                         names[data.selectedMaterialIndex];
    //                       materialComponent->material =
    //                         sl::MaterialManager::get().acquire(materialName);
    //                   } else {
    //                       materialComponent->material = nullptr;
    //                   }
    //               }
    //           }
    //         );
    //         sl::ui::separator();
    //     }

    //     if (entity->hasComponent<sl::TransformComponent>()) {
    //         auto component = entity->getComponent<sl::TransformComponent>();
    //         sl::ui::treeNode(
    //           ICON_FA_STREET_VIEW "  Transform",
    //           [&]([[maybe_unused]] bool) {
    //               auto& transform = component->transform;
    //               sl::ui::namedScope("transform-component-ui", [&]() {
    //                   m_translationSlider.render([&](const sl::Vec3<sl::f32>&
    //                   data) {
    //                       transform.setPosition(data);
    //                   });

    //                   m_scaleSlider.render([&](const sl::Vec3<sl::f32>& data) {
    //                       transform.setScale(data);
    //                   });

    //                   m_orientationSlider.render([&](const sl::Vec3<sl::f32>&
    //                   data) {
    //                       transform.setRotation(sl::math::orientate4(data));
    //                   });
    //               });
    //           }
    //         );
    //         sl::ui::separator();
    //     }

    //     if (entity->hasComponent<sl::PointLightComponent>()) {
    //         auto component = entity->getComponent<sl::PointLightComponent>();
    //         sl::ui::treeNode(
    //           ICON_FA_LIGHTBULB "  Point Light",
    //           [&]([[maybe_unused]] bool) {
    //               sl::ui::namedScope("point-light-component-ui", [&]() {
    //                   ImGui::ColorPicker3("Color",
    //                   (float*)&component->light.color);
    //                   //   ImGui::Sli
    //               });
    //           }
    //         );
    //         sl::ui::separator();
    //     }
    // });
}

#include "MeshCompositeUI.hh"

#include <starlight/ui/UI.hh>
#include <starlight/core/event/EventProxy.hh>
#include <starlight/renderer/factories/MeshFactory.hh>
#include <starlight/renderer/factories/MaterialFactory.hh>

#include "Events.hh"

namespace sle {

MeshCompositeUI::MeshCompositeUI(Resources& resources) : m_resources(resources) {}

void MeshCompositeUI::renderInstanceUI(sl::Transform& instance) {
    auto position = instance.getPosition();
    if (sl::ui::slider("Position", position, { -10.0f, 10.0f, 0.01f })) {
        instance.setPosition(position);
    }
    auto scale = instance.getScale();
    if (sl::ui::slider("Scale", scale, { -5.0f, 5.0f, 0.01f })) {
        instance.setScale(scale);
    }
}

void MeshCompositeUI::renderNodeUI(sl::MeshComposite::Node& node) {
    sl::ui::combo(
      "Mesh", node.mesh.getName(), sl::MeshFactory::get().getAll(),
      [&](auto& mesh) { node.mesh = mesh; }
    );
    sl::ui::combo(
      "Material", node.material.getName(), sl::MaterialFactory::get().getAll(),
      [&](auto& material) { node.material = material; }
    );
    if (sl::ui::button("Add Instance")) node.addInstance();
}

void MeshCompositeUI::renderMeshCompositeUI([[maybe_unused]] sl::MeshComposite& mesh
) {}

bool MeshCompositeUI::renderSceneNode(sl::MeshComposite& component) {
    const auto treeFlags =
      ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen;
    bool clicked = false;

    auto uiOnClick = [&](auto&& callback) {
        if (sl::ui::wasItemClicked()) {
            clicked = true;
            sl::EventProxy::get().emit<events::SetComponentUICallback>(
              std::move(callback)
            );
        }
    };

    sl::ui::treeNode(
      ICON_FA_CUBES "  MeshComposite",
      [&]() {
          uiOnClick([&]() { renderMeshCompositeUI(component); });

          component.traverse([&](auto& node) {
              sl::ui::treeNode(
                fmt::format("{}  {}", ICON_FA_CUBE, node.name),
                [&]() {
                    uiOnClick([&]() { renderNodeUI(node); });

                    auto instances = node.getInstances();
                    for (sl::u64 i = 0; i < instances.size(); ++i) {
                        auto& instance          = instances[i];
                        const auto instanceName = fmt::format("Instance_{}", i);
                        ImGui::BulletText("%s", instanceName.c_str());
                        uiOnClick([&]() { renderInstanceUI(instance); });
                    }
                },
                treeFlags
              );
          });
      },
      treeFlags
    );

    return clicked;
}

}  // namespace sle

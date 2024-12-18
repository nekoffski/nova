#include "MeshCompositeUI.hh"

#include <starlight/ui/UI.hh>

namespace sle {

bool MeshCompositeUI::renderSceneNode(sl::MeshComposite& component) {
    const auto treeFlags =
      ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen;

    sl::ui::treeNode(
      ICON_FA_CUBES "  MeshComposite",
      [&]() {
          component.traverse([&](auto& node) {
              sl::ui::treeNode(
                fmt::format("{}  {}", ICON_FA_CUBE, node.name),
                [&]() {
                    auto instances = node.getInstances();
                    for (sl::u64 i = 0; i < instances.size(); ++i) {
                        const auto instanceName = fmt::format("Instance_{}", i);
                        ImGui::BulletText("%s", instanceName.c_str());
                    }
                },
                treeFlags
              );
          });
      },
      treeFlags
    );

    return false;
}

}  // namespace sle

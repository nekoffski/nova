#include "ResourcesView.hh"

namespace sle {

ResourcesView::ResourcesView(Resources& resources
) : m_resources(resources), m_tabMenu("resource-view-tab-menu") {
    m_tabMenu.addTab("Textures", [&]() { renderTexturesTab(); })
      .addTab("Materials", [&]() { renderMaterialsTab(); })
      .addTab("Meshes", [&]() { renderMeshesTab(); });
}

void ResourcesView::render() { m_tabMenu.render(); }

void ResourcesView::renderMeshesTab() {
    for (auto& mesh : m_resources.meshes) {
        sl::ui::text("{}", mesh.getName());
    }
}

void ResourcesView::renderMaterialsTab() {
    for (auto& material : m_resources.materials) {
        sl::ui::text("{}", material.getName());
    }
}

void ResourcesView::renderTexturesTab() {}

}  // namespace sle

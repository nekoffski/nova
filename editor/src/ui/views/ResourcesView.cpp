#include "ResourcesView.hh"

namespace sle {

ResourcesView::ResourcesView(Resources& resources
) : m_resources(resources), m_tabMenu("resource-view-tab-menu") {
    m_tabMenu.addTab("Materials", [&]() { renderMaterialsTab(); })
      .addTab("Textures", [&]() { renderTexturesTab(); })
      .addTab("Meshes", [&]() { renderMeshesTab(); });
}

void ResourcesView::render() { m_tabMenu.render(); }

void ResourcesView::renderMeshesTab() {
    for (auto& mesh : m_resources.meshes) {
        sl::ui::text("{}", mesh.getName());
    }
}

void ResourcesView::renderMaterialsTab() {
    if (sl::ui::button("Create new material")) {
    }

    sl::ui::separator();

    for (auto& material : m_resources.materials) {
        const auto x = ImGui::GetWindowWidth() / 6.0f;
        sl::ui::group([&]() {
            getImageHandle(material->getProperties().diffuseMap)
              ->show({ x, x }, { 0, 0 }, { 1.0f, 1.0f });
            sl::ui::text("{}", material.getName());
        });
        sl::ui::sameLine();
    }
}

sl::ui::ImageHandle* ResourcesView::getImageHandle(sl::Texture* texture) {
    auto id     = texture->getId();
    auto record = m_data.textures.find(id);

    if (record != m_data.textures.end()) return record->second.get();

    m_data.textures[id] = sl::ui::ImageHandle::createHandle(texture);
    return m_data.textures.at(id).get();
}

void ResourcesView::renderTexturesTab() {}

}  // namespace sle

#include "ResourcesView.hh"

#include <starlight/core/event/EventProxy.hh>
#include <starlight/renderer/Material.hh>

#include "Console.hh"
#include "Events.hh"

namespace sle {

static constexpr sl::u32 rowSize = 5;

static sl::f32 getThumbnailWidth() {
    return ImGui::GetWindowWidth() / static_cast<sl::f32>(rowSize + 1);
}

// TODO: add concepts
static void renderResourceTab(
  const std::string& name, auto& container, auto&& createNew, auto&& render,
  auto&& renderThumbnail
) {
    if (sl::ui::button(fmt::format("Create new {}", name))) {
        auto resource = createNew();
        container.push_back(resource);
        sl::EventProxy::get().emit<events::SetResourceUICallback>(
          [&, render, resource = resource]() { render(resource); }
        );
    }

    sl::ui::separator();
    const auto width = getThumbnailWidth();

    for (sl::u64 i = 0; i < container.size(); ++i) {
        if (i % rowSize != 0) sl::ui::sameLine();

        auto& resource = container[i];

        sl::ui::group([&]() {
            renderThumbnail(resource, width);
            sl::ui::text("{}", resource.getName());
        });

        if (sl::ui::wasItemClicked()) {
            EDITOR_LOG_DEBUG("{} selected: {}", name, resource.getName());
            sl::EventProxy::get().emit<events::SetResourceUICallback>([&, render]() {
                render(resource);
            });
        }
    }
}

ResourcesView::ResourcesView(Resources& resources) :
    m_resources(resources), m_tabMenu("resource-view-tab-menu"),
    m_materialUI(resources) {
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
    renderResourceTab(
      "Material", m_resources.materials, []() { return sl::Material::create(); },
      [&](auto& material) { m_materialUI.render(material); },
      [&](auto& material, const auto width) {
          m_resources.getImageHandle(material->getProperties().diffuseMap)
            .show({ width, width }, { 0, 0 }, { 1.0f, 1.0f });
      }
    );
}

void ResourcesView::renderTexturesTab() {
    renderResourceTab(
      "Texture", m_resources.textures, []() { return sl::Texture::create(); },
      [&](auto& texture) { m_textureUI.render(texture); },
      [&](auto& texture, const auto width) {
          m_resources.getImageHandle(texture).show(
            { width, width }, { 0, 0 }, { 1.0f, 1.0f }
          );
      }
    );
}

}  // namespace sle
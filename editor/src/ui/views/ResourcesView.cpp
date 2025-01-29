#include "ResourcesView.hh"

#include <starlight/core/event/EventProxy.hh>
#include <starlight/renderer/Material.hh>
#include <starlight/renderer/factories/MeshFactory.hh>
#include <starlight/renderer/factories/MaterialFactory.hh>
#include <starlight/renderer/factories/TextureFactory.hh>

#include "Console.hh"
#include "Events.hh"

namespace sle {

static constexpr sl::u32 rowSize = 5;

static sl::f32 getThumbnailWidth() {
    return ImGui::GetWindowWidth() / static_cast<sl::f32>(rowSize + 1);
}

// TODO: add concepts
static void renderResourceTab(
  const std::string& name, auto& manager, auto&& create, auto&& render,
  auto&& renderThumbnail
) {
    if (sl::ui::button(fmt::format("Create new {}", name))) {
        sl::EventProxy::get().emit<events::SetResourceUICallback>(
          [&, render, resource = create()]() { render(resource); }
        );
    }

    sl::ui::separator();
    const auto width = getThumbnailWidth();

    auto resources = manager.getAll();

    for (sl::u64 i = 0; i < resources.size(); ++i) {
        if (i % rowSize != 0) sl::ui::sameLine();

        sl::ui::group([&, resource = resources[i]]() {
            renderThumbnail(resource, width);
            sl::ui::text("{}", resource.getName());
        });

        if (sl::ui::wasItemClicked()) {
            editorWriteDebug("{} selected: {}", name, resources[i].getName());
            sl::EventProxy::get().emit<events::SetResourceUICallback>(
              [render = std::move(render), resource = resources[i]]() {
                  render(resource);
              }
            );
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
    for (auto& mesh : sl::MeshFactory::get().getAll()) {
        sl::ui::text("{}", mesh.getName());
    }
}

void ResourcesView::renderMaterialsTab() {
    renderResourceTab(
      "Material", sl::MaterialFactory::get(),
      [&]() { return m_resources.addMaterial(); },
      [&](auto& material) { m_materialUI.render(material); },
      [&](auto& material, const auto width) {
          m_resources.getImageHandle(material->getTextures().diffuseMap)
            .show({ width, width }, { 0, 0 }, { 1.0f, 1.0f });
      }
    );
}

void ResourcesView::renderTexturesTab() {
    renderResourceTab(
      "Texture", sl::TextureFactory::get(),
      [&]() { return m_resources.addTexture(); },
      [&](auto& texture) { m_textureUI.render(texture); },
      [&](auto& texture, const auto width) {
          m_resources.getImageHandle(texture).show(
            { width, width }, { 0, 0 }, { 1.0f, 1.0f }
          );
      }
    );
}

}  // namespace sle
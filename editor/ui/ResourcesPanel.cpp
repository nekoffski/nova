#include "ResourcesPanel.hh"

#include <thread>

template <typename Resource, typename Manager>
void renderResourceTab(Manager* manager, ResourceType resourceType, UIState* state) {
    manager->forEach([&](const std::string& name, const Resource* resource) {
        const auto resourceId = resource->getId();

        const auto color =
          (state->selectedResourceId && *state->selectedResourceId == resourceId
           && resourceType == state->selectedResourceType)
            ? selectedColor
            : defaultColor;

        sl::ui::withColor(color, [&]() {
            if (sl::ui::text("{}", name)) {
                state->selectedResourceId   = resourceId;
                state->selectedResourceType = resourceType;
            }
        });
    });
}

ResourcesPanel::ResourcesPanel(UIState* state, Logger* logger) :
    m_state(state), m_logger(logger), m_resourcesTab("Resources"),
    m_loadPopup("load-popup", [&]() { renderLoadPopup(); }) {
    m_resourcesTab
      .addTab(
        ICON_FA_IMAGE "  Textures",
        [&]() {
            renderResourceTab<sl::Texture>(
              sl::TextureManager::getPtr(), ResourceType::texture, m_state
            );
        }
      )
      .addTab(ICON_FA_PLANE "  Models", [&]() {})
      .addTab(
        ICON_FA_PROJECT_DIAGRAM "  Meshes",
        [&]() {
            renderResourceTab<sl::Mesh>(
              sl::MeshManager::getPtr(), ResourceType::mesh, m_state
            );
        }
      )
      .addTab(
        ICON_FA_GLOBE_AMERICAS "  Materials",
        [&]() {
            renderResourceTab<sl::Material>(
              sl::MaterialManager::getPtr(), ResourceType::material, m_state
            );
        }
      )
      .addTab(ICON_FA_SUN "  Shaders", [&]() {});
}

void ResourcesPanel::render() {
    if (sl::ui::button("Add")) m_loadPopup.open();

    sl::ui::sameLine();
    m_resourcesTab.render();
    m_loadPopup.render();
}

void ResourcesPanel::renderLoadPopup() { sl::ui::text("Hello world!"); }

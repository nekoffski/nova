#include "ScenePanel.hh"

#include "starlight/ui/UI.hh"

ScenePanel::ScenePanel(sl::Scene* scene, UIState* state, Logger* logger) :
    m_scene(scene), m_state(state), m_logger(logger), m_tabs("scene-tabs") {
    m_tabs.addTab(
            ICON_FA_SITEMAP "  Scene graph", [&]() { renderSceneGraph(); }
    ).addTab(ICON_FA_WRENCH "  Scene Settings", [&]() { renderSceneSettings(); });
}

void ScenePanel::render() { m_tabs.render(); }

void ScenePanel::renderSceneGraph() {
    if (sl::ui::button("Add Entity")) {
        m_logger->debug("Adding new entity");
        m_scene->addEntity();
    }
    sl::ui::separator();
    sl::ui::treeNode(ICON_FA_CUBE "  Root", [&]([[maybe_unused]] bool clicked) {
        m_scene->forEachEntity(
          [&](const std::string& name, sl::Entity* entity) -> void {
              auto& selectedEntityId = m_state->selectedEntityId;

              const auto entityId = entity->getId();
              const auto color =
                selectedEntityId && *selectedEntityId == entityId
                  ? selectedColor
                  : defaultColor;

              sl::ui::withColor(color, [&]() {
                  const auto title = fmt::format("\t{}  {}", ICON_FA_CUBE, name);

                  if (sl::ui::text(title)) {
                      m_logger->debug("Selected entity with id={}", entityId);
                      selectedEntityId = entityId;
                  }
              });
          }
        );
    });
}

void ScenePanel::renderSceneSettings() {
    sl::ui::checkbox("Show grid", m_state->showGrid);
    sl::ui::separator();
}

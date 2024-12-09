#include "SceneView.hh"

#include <starlight/core/event/EventProxy.hh>
#include "Events.hh"

namespace sle {

SceneView::SceneView(sl::Scene* scene) :
    m_tabMenu("Scene"), m_entitiesTab(scene), m_skyboxTab(scene),
    m_cameraTab(scene) {
    m_tabMenu
      .addTab(
        ICON_FA_CODE_BRANCH "  Entities Tree", [&]() { m_entitiesTab.render(); }
      )
      .addTab(ICON_FA_CLOUD "  Skybox", [&]() { m_skyboxTab.render(); })
      .addTab(ICON_FA_CAMERA "  Camera", [&]() { m_cameraTab.render(); });
}

void SceneView::render() { m_tabMenu.render(); }

SceneView::EntitiesTab::EntitiesTab(sl::Scene* scene
) : SceneTab(scene), m_selectedEntity(nullptr) {}

void SceneView::EntitiesTab::render() {
    auto& eventProxy = sl::EventProxy::get();

    if (sl::ui::button("Add Entity", sl::ui::parentWidth)) {
        auto& entity = m_scene->addEntity();
        EDITOR_LOG_INFO("New entity added: {}/{}", entity.getId(), entity.getName());
    }
    sl::ui::separator();
    sl::ui::treeNode(
      "Root",
      [&]() {
          m_scene->forEachEntity([&](sl::Entity& entity) {
              auto flags =
                ImGuiTreeNodeFlags_OpenOnDoubleClick
                | ImGuiTreeNodeFlags_DefaultOpen;

              if (m_selectedEntity && m_selectedEntity->getId() == entity.getId()) {
                  flags |= ImGuiTreeNodeFlags_Selected;
              }

              sl::ui::treeNode(
                entity.getName(),
                [&]() {
                    // TODO: display child entitites
                },
                flags
              );

              if (sl::ui::wasItemClicked()) {
                  EDITOR_LOG_DEBUG("Entity selected: {}", entity.getName());
                  m_selectedEntity = &entity;
                  eventProxy.emit<events::EntitySelected>(&entity);
              }
          });
      },
      ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen
    );
}

SceneView::SceneTab::SceneTab(sl::Scene* scene) : m_scene(scene) {}

void SceneView::SkyboxTab::render() {}

void SceneView::CameraTab::render() {}

}  // namespace sle

#include "SceneView.hh"

#include <starlight/core/event/EventProxy.hh>

#include "components/PointLightUI.hh"
#include "components/DirectionalLightUI.hh"
#include "components/MeshCompositeUI.hh"
#include "Events.hh"

namespace sle {

SceneView::SceneView(sl::Scene* scene, Resources& resources) :
    m_tabMenu("Scene"), m_entitiesTab(scene, resources), m_skyboxTab(scene),
    m_cameraTab(scene) {
    m_tabMenu
      .addTab(
        ICON_FA_CODE_BRANCH "  Entities Tree", [&]() { m_entitiesTab.render(); }
      )
      .addTab(ICON_FA_CLOUD "  Skybox", [&]() { m_skyboxTab.render(); })
      .addTab(ICON_FA_CAMERA "  Camera", [&]() { m_cameraTab.render(); });
}

void SceneView::render() { m_tabMenu.render(); }

SceneView::EntitiesTab::EntitiesTab(sl::Scene* scene, Resources& resources) :
    SceneTab(scene), m_selectedEntity(nullptr) {
    addComponentUI<PointLightUI>();
    addComponentUI<DirectionalLightUI>();
    addComponentUI<MeshCompositeUI>(resources);
}

void SceneView::EntitiesTab::render() {
    if (sl::ui::button("Add Entity", sl::ui::parentWidth)) {
        auto& entity = m_scene->addEntity();
        editorWriteInfo("New entity added: {}/{}", entity.getId(), entity.name);
    }

    sl::ui::separator();
    sl::ui::treeNode(
      "Root",
      [&]() {
          m_scene->forEachEntity([&](sl::Entity& entity) {
              auto flags =
                ImGuiTreeNodeFlags_OpenOnDoubleClick
                | ImGuiTreeNodeFlags_DefaultOpen;

              if (m_selectedEntity && m_selectedEntity->getId() == entity.getId())
                  flags |= ImGuiTreeNodeFlags_Selected;

              sl::ui::treeNode(
                entity.name,
                [&]() {
                    if (sl::ui::wasItemClicked()) {
                        const auto clearComponentCallback =
                          not m_selectedEntity
                          || m_selectedEntity->getId() != entity.getId();
                        selectEntity(entity, clearComponentCallback);
                    }

                    for (const auto& componentIndex : entity.getComponentTypes()) {
                        if (m_componentUIs.contains(componentIndex)) {
                            bool entityClicked =
                              m_componentUIs[componentIndex]->renderSceneNode(
                                entity.getComponent(componentIndex)
                              );
                            if (entityClicked) selectEntity(entity, false);
                        }
                    }
                    // TODO: display child entitites
                },
                flags
              );
          });
      },
      ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_DefaultOpen
    );
}

void SceneView::EntitiesTab::selectEntity(
  sl::Entity& entity, bool clearComponentCallback
) {
    editorWriteDebug("Entity selected: {}", entity.name);
    m_selectedEntity = &entity;
    sl::EventProxy::get().emit<events::EntitySelected>(
      m_selectedEntity, clearComponentCallback
    );
}

SceneView::SceneTab::SceneTab(sl::Scene* scene) : m_scene(scene) {}

void SceneView::SkyboxTab::render() {}

void SceneView::CameraTab::render() {}

}  // namespace sle

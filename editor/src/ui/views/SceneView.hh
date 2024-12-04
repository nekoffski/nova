#pragma once

#include <starlight/ui/UI.hh>
#include <starlight/scene/Scene.hh>

#include "Console.hh"

namespace sle {

class SceneView {
    class SceneTab {
    public:
        explicit SceneTab(sl::Scene* scene);

    protected:
        sl::Scene* m_scene;
    };

    class EntitiesTab : public SceneTab {
    public:
        explicit EntitiesTab(sl::Scene* scene);
        void render();

    private:
        sl::Entity* m_selectedEntity;
    };

    class SkyboxTab : public SceneTab {
    public:
        using SceneTab::SceneTab;
        void render();
    };

    class CameraTab : public SceneTab {
    public:
        using SceneTab::SceneTab;
        void render();
    };

public:
    explicit SceneView(sl::Scene* scene);

    void render();

private:
    sl::ui::TabMenu m_tabMenu;

    EntitiesTab m_entitiesTab;
    SkyboxTab m_skyboxTab;
    CameraTab m_cameraTab;
};

}  // namespace sle

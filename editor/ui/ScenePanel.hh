#pragma once

#include "starlight/scene/Scene.hh"
#include "starlight/ui/UI.hh"

#include "Core.hh"
#include "Console.hh"

class ScenePanel {
public:
    explicit ScenePanel(sl::Scene* scene, UIState* state, Logger* logger);

    void render();

private:
    void renderSceneGraph();
    void renderSceneSettings();

    sl::Scene* m_scene;
    UIState* m_state;
    Logger* m_logger;

    sl::ui::TabMenu m_tabs;
};

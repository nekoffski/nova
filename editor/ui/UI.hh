#pragma once

#include <optional>

#include "starlight/core/Core.hh"
#include "starlight/ui/UI.hh"

#include "starlight/renderer/RendererFrontend.hh"

#include "starlight/scene/Scene.hh"

#include "ScenePanel.hh"
#include "EntityInspectorPanel.hh"
#include "ResourceInspectorPanel.hh"
#include "ResourcesPanel.hh"
#include "Console.hh"

#include "Core.hh"

class UI {
public:
    explicit UI(
      sl::u64 w, sl::u64 h, sl::RendererFrontend& renderer, sl::Scene* scene
    );

    void setScene(sl::Scene* scene);

    void render();
    bool shouldExit() const;

    UIState* getState();
    Logger* getLogger();

private:
    sl::u64 m_width;
    sl::u64 m_height;

    sl::Scene* m_scene;

    Console m_console;
    Logger* m_logger;

    sl::ui::MainMenuBar m_mainMenu;

    sl::ui::PanelCombo m_leftCombo;
    sl::ui::PanelCombo m_rightCombo;
    sl::ui::PanelCombo m_bottomCombo;

    UIState m_state;

    ScenePanel m_scenePanel;
    ResourceInspectorPanel m_resourceInspectorPanel;
    EntityInspectorPanel m_entityInspectorPanel;
    ResourcesPanel m_resourcesPanel;

    bool m_shouldExit;
};

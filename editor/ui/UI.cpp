#include "UI.hh"

#include "starlight/core/Log.hh"
#include "starlight/core/math/Core.hh"
#include "starlight/core/event/Event.hh"

#include "starlight/ui/fonts/FontAwesome.hh"

#include "Events.h"

UI::UI(sl::u64 w, sl::u64 h, sl::RendererFrontend& renderer, sl::Scene* scene) :
    m_width(w), m_height(h), m_scene(scene), m_logger(m_console.getLogger()),
    m_leftCombo(
      "left-combo",
      sl::ui::PanelCombo::Properties{
        .position             = { 0,                    0 },
        .size                 = { panelWidthFactor * w, h },
        .alignWithMainMenuBar = true,
        .orientation          = sl::ui::PanelCombo::Orientation::vertical
}
    ),
    m_rightCombo(
      "right-combo",
      sl::ui::PanelCombo::Properties{
        .position             = { w * (1.0f - panelWidthFactor), 0 },
        .size                 = { panelWidthFactor * w, h },
        .alignWithMainMenuBar = true,
        .orientation          = sl::ui::PanelCombo::Orientation::vertical }
    ),
    m_bottomCombo(
      "bottom-combo",
      sl::ui::PanelCombo::Properties{
        .position             = { panelWidthFactor * w, (1.0f - 0.25f) * h },
        .size                 = { (1.0f - panelWidthFactor * 2) * w, h * 0.25f },
        .alignWithMainMenuBar = true,
        .orientation          = sl::ui::PanelCombo::Orientation::horizontal }

    ),
    m_scenePanel(m_scene, &m_state, m_logger),
    m_resourceInspectorPanel(m_scene, &m_state, m_logger),
    m_entityInspectorPanel(m_scene, &m_state, m_logger),
    m_resourcesPanel(&m_state, m_logger), m_shouldExit(false) {
    m_mainMenu.addMenu("File")
      .addItem("New", []() {})
      .addItem(
        "Load", "Ctrl+O",
        []() { sl::EventManager::get().emit<SceneLoaded>("scene.json"); }
      )
      .addItem("Save", "Ctrl+S", []() {})
      .addItem(
        "Save as", []() { sl::EventManager::get().emit<SceneSaved>("scene.json"); }
      )
      .addItem("Quit", [&]() { m_shouldExit = true; });
    m_mainMenu.addMenu("Help");

    m_bottomCombo
      .addPanel(ICON_FA_FILE "  Resources", [&]() { m_resourcesPanel.render(); })
      .addPanel(ICON_FA_TERMINAL "  Console", [&]() { m_console.render(); });

    m_leftCombo.addPanel(ICON_FA_CITY "  Scene", [&]() { m_scenePanel.render(); })
      .addPanel(ICON_FA_CHART_LINE "  Statistics", [&]() {
          const auto [renderedVertices, frameNumber, delta] =
            renderer.getFrameStatistics();
          sl::ui::text("Frame time: {}", delta);
          sl::ui::text("Frames per second: {}", int(1.0f / delta));
          sl::ui::text("Frame number: {}", frameNumber);
          sl::ui::separator();
          sl::ui::text("Rendered vertices: {}", renderedVertices);
          sl::ui::separator();
          sl::ui::text("8/9/0 - change render mode");
          sl::ui::text("6     - dump var logs");
          sl::ui::text("3/4   - switch camera");
          sl::ui::text("u     - on/off update");
      });

    m_rightCombo
      .addPanel(
        ICON_FA_CUBE "  Entity Inspector", [&]() { m_entityInspectorPanel.render(); }
      )
      .addPanel(ICON_FA_FILE "  Resource Inspector", [&]() {
          m_resourceInspectorPanel.render();
      });

    m_logger->debug("UI initialized");
}

void UI::setScene(sl::Scene* scene) { m_scene = scene; }

void UI::render() {
    m_mainMenu.render();
    m_leftCombo.render();
    m_rightCombo.render();
    m_bottomCombo.render();
}

bool UI::shouldExit() const { return m_shouldExit; }

UIState* UI::getState() { return &m_state; }

Logger* UI::getLogger() { return m_logger; }

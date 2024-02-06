#include "UI.h"

#include "starlight/core/utils/Log.h"
#include "starlight/core/math/Glm.h"

#include "starlight/ui/fonts/FontAwesome.h"

#include "starlight/scene/components/MeshComponent.h"
#include "starlight/scene/components/TransformComponent.h"

UI::UI(sl::u64 w, sl::u64 h, sl::RendererFrontend& renderer, sl::Scene* scene) :
    m_width(w), m_height(h), m_scene(scene),
    m_leftCombo(
      "left-combo",
      sl::ui::PanelCombo::Properties{
        .position             = {0,                         0},
        .size                 = { leftComboWidthFactor * w, h},
        .alignWithMainMenuBar = true,
}
    ),
    m_rightCombo(
      "right-combo",
      sl::ui::PanelCombo::Properties{
        .position             = { w * (1.0f - leftComboWidthFactor), 0 },
        .size                 = { leftComboWidthFactor * w, h },
        .alignWithMainMenuBar = true,
      }
    ),
    m_bottomCombo(
      "bottom-combo",
      sl::ui::PanelCombo::Properties{
        .position             = { leftComboWidthFactor * w, (1.0f - 0.25f) * h },
        .size                 = { (1.0f - leftComboWidthFactor * 2) * w, h * 0.25f },
        .alignWithMainMenuBar = true,
      }
    ),
    m_scenePanel(m_scene, &m_state), m_inspectorPanel(m_scene, &m_state),
    m_resourcesPanel(&m_state), m_shouldExit(false) {
    m_mainMenu.addMenu("File")
      .addItem("Create", []() {})
      .addItem("Open", "Ctrl+O", []() {})
      .addItem("Save", "Ctrl+S", []() {})
      .addItem("Save as", []() {})
      .addItem("Quit", [&]() { m_shouldExit = true; });
    m_mainMenu.addMenu("Help");

    m_bottomCombo.addPanel(ICON_FA_FILE "  Resources", [&]() {
        m_resourcesPanel.render();
    });

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

    m_rightCombo.addPanel(ICON_FA_EYE "  Inspector", [&]() {
        m_inspectorPanel.render();
    });
}

void UI::setScene(sl::Scene* scene) { m_scene = scene; }

void UI::render() {
    m_mainMenu.render();
    m_leftCombo.render();
    m_rightCombo.render();
    m_bottomCombo.render();
}

bool UI::shouldExit() const { return m_shouldExit; }
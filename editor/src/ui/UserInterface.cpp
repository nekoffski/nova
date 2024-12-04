#include "UserInterface.hh"

#include <starlight/ui/fonts/FontAwesome.hh>

namespace sle {

static sl::ui::PanelCombo::Properties createLeftComboProperties(
  const sl::Vec2<sl::u32>& viewport, const UserInterface::Config& config
) {
    return sl::ui::PanelCombo::Properties{
        .position             = { 0,                                   0          },
        .size                 = { config.panelWidthRatio * viewport.x, viewport.y },
        .alignWithMainMenuBar = true,
        .orientation          = sl::ui::PanelCombo::Orientation::vertical,
    };
}

static sl::ui::PanelCombo::Properties createBottomComboProperties(
  const sl::Vec2<sl::u32>& viewport, const UserInterface::Config& config
) {
    return sl::ui::PanelCombo::Properties{
        .position             = { config.panelWidthRatio * viewport.x,
                                 (1.0f - config.panelHeightRatio) * viewport.y },
        .size                 = { (1.0f - config.panelWidthRatio) * viewport.x,
                                 viewport.y * config.panelHeightRatio          },
        .alignWithMainMenuBar = true,
        .orientation          = sl::ui::PanelCombo::Orientation::horizontal,
    };
}

UserInterface::Config UserInterface::Config::createDefault() {
    return Config{ .panelWidthRatio = 0.20f, .panelHeightRatio = 0.25f };
}

UserInterface::UserInterface(
  const sl::Vec2<sl::u32>& viewport, sl::Scene* scene, sl::RenderGraph* renderGraph,
  const Config& config
) :
    m_viewport(viewport),
    m_leftCombo("left-combo", createLeftComboProperties(viewport, config)),
    m_bottomCombo("bottom-combo", createBottomComboProperties(viewport, config)),
    m_sceneView(scene), m_inspectorView(renderGraph) {
    initMenu();
    initLeftCombo();
    initBottomCombo();

    EDITOR_LOG_INFO("UI started!");
    EDITOR_LOG_INFO("Welcome to the Starlight Engine Editor");
}

void UserInterface::onViewportReisze(const sl::Vec2<sl::u32>& viewport) {
    m_viewport = viewport;
}

void UserInterface::setRenderGraph(sl::RenderGraph& renderGraph) {
    m_inspectorView.setRenderGraph(renderGraph);
}

void UserInterface::render() {
    m_menu.render();
    m_leftCombo.render();
    m_bottomCombo.render();
}

void UserInterface::initBottomCombo() {
    m_bottomCombo
      .addPanel(
        ICON_FA_FOLDER "  Resources", [&]() { sl::ui::text("Hello world!"); }
      )
      .addPanel(ICON_FA_TERMINAL "  Messages", [&]() {
          sl::ui::namedScope("console-content", [&]() {
              sl::ui::text("{}", m_console.getBuffer());
          });
      });
}

void UserInterface::initLeftCombo() {
    m_leftCombo.addPanel(ICON_FA_CITY "  Scene", [&]() { m_sceneView.render(); })
      .addPanel(ICON_FA_WRENCH "  Inspector", [&]() { m_inspectorView.render(); });
}

void UserInterface::initMenu() {
    m_menu.addMenu("File").addItem("Exit", []() {});
    m_menu.addMenu("Help").addItem("Show help", []() {});
}
}  // namespace sle

#include "UserInterface.hh"

#include <starlight/ui/fonts/FontAwesome.hh>
#include <starlight/window/Events.hh>

#include "Events.hh"

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
    m_eventSentinel(sl::EventProxy::get()), m_config(config), m_viewport(viewport),
    m_sceneView(scene, m_resources), m_propertiesView(renderGraph),
    m_resourcesView(m_resources) {
    m_eventSentinel.add<sl::WindowResized>([&](auto& event) {
        onViewportReisze(event.size);
    });

    createLayout(viewport);
    initMenu();

    editorWriteInfo("UI started!");
    editorWriteInfo("Welcome to the Starlight Engine Editor");
}

void UserInterface::createLayout(const sl::Vec2<sl::u32>& viewport) {
    m_bottomCombo.clear();
    m_leftCombo.clear();

    m_leftCombo.emplace("left-combo", createLeftComboProperties(viewport, m_config));
    m_bottomCombo
      .emplace("bottom-combo", createBottomComboProperties(viewport, m_config));

    initLeftCombo();
    initBottomCombo();
}

void UserInterface::onViewportReisze(const sl::Vec2<sl::u32>& viewport) {
    m_viewport = viewport;
    createLayout(viewport);
}

void UserInterface::setRenderGraph(sl::RenderGraph& renderGraph) {
    m_propertiesView.setRenderGraph(renderGraph);
}

void UserInterface::render() {
    m_menu.render();
    m_leftCombo->render();
    m_bottomCombo->render();
}

const UserInterface::Config& UserInterface::getConfig() const { return m_config; }

void UserInterface::initBottomCombo() {
    (*m_bottomCombo)
      .addPanel(ICON_FA_FOLDER "  Resources", [&]() { m_resourcesView.render(); })
      .addPanel(ICON_FA_TERMINAL "  Messages", [&]() {
          sl::ui::namedScope("console-content", [&]() {
              sl::ui::text("{}", m_console.getBuffer());
          });
      });
}

void UserInterface::initLeftCombo() {
    (*m_leftCombo)
      .addPanel(ICON_FA_CITY "  Scene", [&]() { m_sceneView.render(); })
      .addPanel(ICON_FA_WRENCH "  Properties", [&]() { m_propertiesView.render(); });
}

void UserInterface::initMenu() {
    m_menu.addMenu("File").addItem("Exit", [&]() {
        editorWriteInfo("File.Exit presesed, emitting quit event");
        sl::EventProxy::get().emit<sl::QuitEvent>("UI.File.Exit pressed");
    });

    static std::string scenePath = "./test.starscene.json";

    m_menu.addMenu("Scene")
      .addItem(
        "Load",
        [&]() {
            editorWriteDebug("Requesting scene load: {}", scenePath);
            sl::EventProxy::get().emit<events::SceneSerialization>(
              events::SceneSerialization::Action::deserialize, scenePath
            );
        }
      )
      .addItem("Save", [&]() {
          editorWriteDebug("Requesting scene save: {}", scenePath);
          sl::EventProxy::get().emit<events::SceneSerialization>(
            events::SceneSerialization::Action::serialize, scenePath
          );
      });

    m_menu.addMenu("Help").addItem("Show help", []() {});
}
}  // namespace sle

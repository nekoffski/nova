#include "UserInterface.hh"

#include <starlight/ui/fonts/FontAwesome.hh>
#include <starlight/core/event/WindowResized.hh>
#include <starlight/core/event/Quit.hh>

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
  sl::EventProxy& eventProxy, const sl::Vec2<sl::u32>& viewport, sl::Scene* scene,
  sl::RenderGraph* renderGraph, const Config& config
) :
    m_eventProxy(eventProxy), m_eventSentinel(eventProxy), m_config(config),
    m_viewport(viewport),
    m_leftCombo("left-combo", createLeftComboProperties(viewport, config)),
    m_bottomCombo("bottom-combo", createBottomComboProperties(viewport, config)),
    m_sceneView(scene, m_resources), m_inspectorView(m_resources, renderGraph),
    m_resourcesView(m_resources) {
    m_eventSentinel.add<sl::WindowResized>([&](auto& event) {
        onViewportReisze(event.size);
        return sl::EventChainBehaviour::propagate;
    });

    initMenu();
    initLeftCombo();
    initBottomCombo();

    EDITOR_LOG_INFO("UI started!");
    EDITOR_LOG_INFO("Welcome to the Starlight Engine Editor");
}

void UserInterface::onViewportReisze(const sl::Vec2<sl::u32>& viewport) {
    m_viewport = viewport;
    // todo: resize everything
}

void UserInterface::setRenderGraph(sl::RenderGraph& renderGraph) {
    m_inspectorView.setRenderGraph(renderGraph);
}

void UserInterface::render() {
    m_menu.render();
    m_leftCombo.render();
    m_bottomCombo.render();
}

const UserInterface::Config& UserInterface::getConfig() const { return m_config; }

void UserInterface::initBottomCombo() {
    m_bottomCombo
      .addPanel(ICON_FA_FOLDER "  Resources", [&]() { m_resourcesView.render(); })
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
    m_menu.addMenu("File").addItem("Exit", [&]() {
        EDITOR_LOG_INFO("File.Exit presesed, emitting quit event");
        m_eventProxy.emit<sl::QuitEvent>("UI.File.Exit pressed");
    });

    static std::string scenePath = "./test.starscene.json";

    m_menu.addMenu("Scene")
      .addItem(
        "Load",
        [&]() {
            EDITOR_LOG_DEBUG("Requesting scene load: {}", scenePath);
            m_eventProxy.emit<events::SceneSerialization>(
              events::SceneSerialization::Action::deserialize, scenePath
            );
        }
      )
      .addItem("Save", [&]() {
          EDITOR_LOG_DEBUG("Requesting scene save: {}", scenePath);
          m_eventProxy.emit<events::SceneSerialization>(
            events::SceneSerialization::Action::serialize, scenePath
          );
      });

    m_menu.addMenu("Help").addItem("Show help", []() {});
}
}  // namespace sle

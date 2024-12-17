#pragma once

#include <starlight/ui/UI.hh>
#include <starlight/scene/Scene.hh>

#include "views/SceneView.hh"
#include "views/InspectorView.hh"
#include "views/ResourcesView.hh"
#include "Console.hh"
#include "Resources.hh"

namespace sle {

class UserInterface {
public:
    struct Config {
        static Config createDefault();

        sl::f32 panelWidthRatio;
        sl::f32 panelHeightRatio;
    };

    explicit UserInterface(
      sl::EventProxy& eventProxy, const sl::Vec2<sl::u32>& viewport,
      sl::Scene* scene, sl::RenderGraph* renderGraph = nullptr,
      const Config& config = Config::createDefault()
    );

    void onViewportReisze(const sl::Vec2<sl::u32>& viewport);
    void setRenderGraph(sl::RenderGraph& renderGraph);

    void render();

    const Config& getConfig() const;

private:
    void initMenu();
    void initLeftCombo();
    void initBottomCombo();

    sl::EventProxy& m_eventProxy;
    sl::EventHandlerSentinel m_eventSentinel;

    Config m_config;

    sl::Vec2<sl::u32> m_viewport;

    Console m_console;
    Resources m_resources;

    sl::ui::MainMenuBar m_menu;
    sl::ui::PanelCombo m_leftCombo;
    sl::ui::PanelCombo m_bottomCombo;

    SceneView m_sceneView;
    InspectorView m_inspectorView;
    ResourcesView m_resourcesView;
};

}  // namespace sle

#pragma once

#include <starlight/ui/UI.hh>

#include "views/SceneView.hh"
#include "views/InspectorView.hh"
#include "Console.hh"

namespace sle {

class UserInterface {
public:
    struct Config {
        static Config createDefault();

        sl::f32 panelWidthRatio;
        sl::f32 panelHeightRatio;
    };

    explicit UserInterface(
      const sl::Vec2<sl::u32>& viewport,
      const Config& config = Config::createDefault()
    );

    void onViewportReisze(const sl::Vec2<sl::u32>& viewport);

    void render();

private:
    void initMenu();
    void initLeftCombo();
    void initBottomCombo();

    sl::Vec2<sl::u32> m_viewport;

    Console m_console;
    Logger* m_logger;

    sl::ui::MainMenuBar m_menu;
    sl::ui::PanelCombo m_leftCombo;
    sl::ui::PanelCombo m_bottomCombo;

    SceneView m_sceneView;
    InspectorView m_inspectorView;
};

}  // namespace sle

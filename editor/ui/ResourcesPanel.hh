#pragma once

#include "Core.hh"
#include "Console.hh"

#include "kc/async/TaskScheduler.hpp"

class ResourcesPanel {
public:
    explicit ResourcesPanel(UIState* state, Logger* logger);

    void render();

private:
    void renderLoadPopup();

    UIState* m_state;
    Logger* m_logger;

    kc::async::TaskScheduler ts;

    sl::ui::TabMenu m_resourcesTab;
    sl::ui::Popup m_loadPopup;
};

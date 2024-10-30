#pragma once

#include "starlight/ui/Core.hh"

namespace sl::ui {

class Panel {
public:
    struct Properties {
        Vec2<f32> position;
        Vec2<f32> size;
    };

    explicit Panel(
      const std::string& name, const Properties& props, Callback&& callback
    );
    void render();

    void setSize(const Vec2<f32>& size);
    void setPosition(const Vec2<f32>& position);

private:
    std::string m_name;
    Properties m_props;
    Callback m_callback;
};

class PanelCombo {
public:
    enum class Orientation { horizontal, vertical };

    struct Properties {
        Vec2<f32> position;
        Vec2<f32> size;
        bool alignWithMainMenuBar;
        Orientation orientation;
    };

    explicit PanelCombo(const std::string& name, const Properties& props);

    void render();

    PanelCombo& addPanel(const std::string& name, Callback&& callback);

    void setSize(const Vec2<f32>& size);
    void setPosition(const Vec2<f32>& position);

private:
    void rebuildPanels();

    std::string m_name;
    Properties m_props;
    std::vector<Panel> m_panels;

    bool m_dirty;
};

}  // namespace sl::ui

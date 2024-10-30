#include "Panel.hh"

#include <imgui.h>

#include "starlight/ui/Utils.hh"

namespace sl::ui {

Panel::Panel(const std::string& name, const Properties& props, Callback&& callback) :
    m_name(name), m_props(props), m_callback(std::move(callback)) {}

void Panel::render() {
    ImGui::SetNextWindowPos({ m_props.position.x, m_props.position.y });
    ImGui::SetNextWindowSize({ m_props.size.x, m_props.size.y });

    if (ImGui::Begin(m_name.c_str())) {
        m_callback();
        ImGui::End();
    }
}

void Panel::setSize(const Vec2<f32>& size) { m_props.size = size; }
void Panel::setPosition(const Vec2<f32>& position) { m_props.position = position; }

PanelCombo::PanelCombo(const std::string& name, const Properties& props) :
    m_name(name), m_props(props), m_dirty(true) {}

void PanelCombo::render() {
    if (m_dirty) rebuildPanels();

    namedScope(m_name, [&]() {
        for (auto& panel : m_panels) panel.render();
    });
}

PanelCombo& PanelCombo::addPanel(const std::string& name, Callback&& callback) {
    m_panels.emplace_back(name, Panel::Properties{}, std::move(callback));
    return *this;
}

void PanelCombo::setSize(const Vec2<f32>& size) { m_props.size = size; }
void PanelCombo::setPosition(const Vec2<f32>& position) {
    m_props.position = position;
}

void PanelCombo::rebuildPanels() {
    const auto panelCount = m_panels.size();

    auto comboHeight           = m_props.size.y;
    auto comboVerticalPosition = m_props.position.y;

    if (m_props.alignWithMainMenuBar) {
        auto frameHeight = getFrameHeight();
        comboHeight -= frameHeight;
        comboVerticalPosition += frameHeight;
    }

    // TODO: unify
    if (m_props.orientation == Orientation::vertical) {
        const float panelHeight = comboHeight / panelCount;

        for (u32 i = 0; i < panelCount; ++i) {
            auto panel = &m_panels[i];

            panel->setPosition(
              { m_props.position.x, comboVerticalPosition + i * panelHeight }
            );
            panel->setSize({ m_props.size.x, panelHeight });
        }
    } else {
        const float panelWidth = m_props.size.x / panelCount;
        for (u32 i = 0; i < panelCount; ++i) {
            auto panel = &m_panels[i];

            panel->setPosition(
              { m_props.position.x + i * panelWidth, m_props.position.y }
            );
            panel->setSize({ panelWidth, m_props.size.y });
        }
    }

    m_dirty = false;
}

}  // namespace sl::ui

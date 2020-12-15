#include "EditorGui.h"

using namespace sl::core;

namespace editor::gui {

EditorGui::EditorGui(const Settings& settings, EntitiesVector& entities, res::ResourceManager& resourceManager)
    : m_leftPanel(settings, entities, resourceManager)
    , m_bottomPanel(settings, resourceManager) {
}

void EditorGui::setSettings(const Settings& settings) {
	m_leftPanel.setSettings(settings);
	m_bottomPanel.setSettings(settings);
}

void EditorGui::renderEditorGui(sl::gui::GuiProxy& gui) {
    m_leftPanel.render(gui);
    m_bottomPanel.render(gui);
}
}
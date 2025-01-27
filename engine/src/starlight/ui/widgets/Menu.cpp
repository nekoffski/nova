#include "Menu.hh"

namespace sl::ui {

Menu::Menu(const std::string& name) : m_name(name) {}

Menu& Menu::addItem(const std::string& name, Callback&& callback) {
    m_items.emplace_back(name, std::nullopt, std::move(callback));
    return *this;
}

Menu& Menu::addItem(
  const std::string& name, const std::string& shortcut, Callback&& callback
) {
    m_items.emplace_back(name, shortcut, std::move(callback));
    return *this;
}

void Menu::render() {
    if (ImGui::BeginMenu(m_name.c_str())) {
        for (const auto& [name, shortcut, callback] : m_items) {
            if (shortcut) {
                if (ImGui::MenuItem(name.c_str(), shortcut->c_str())) callback();
            } else {
                if (ImGui::MenuItem(name.c_str())) callback();
            }
        }
        ImGui::EndMenu();
    }
}
const std::string& Menu::getName() const { return m_name; }

MainMenuBar::MainMenuBar() {
    log::expect(not sCreated, "Only 1 instance of MainMenuBar is allowed!");
    sCreated = true;
}

Menu& MainMenuBar::addMenu(const std::string& name) {
    m_menus.emplace_back(name);
    return m_menus.back();
}

void MainMenuBar::render() {
    if (ImGui::BeginMainMenuBar()) {
        for (auto& menu : m_menus) menu.render();
        ImGui::EndMainMenuBar();
    }
}

}  // namespace sl::ui

#pragma once

#include <starlight/ui/UI.hh>

#include "Resources.hh"

namespace sle {

class ResourcesView {
public:
    explicit ResourcesView(Resources& resources);

    void render();

private:
    void renderTexturesTab();
    void renderMeshesTab();
    void renderMaterialsTab();

    Resources& m_resources;
    sl::ui::TabMenu m_tabMenu;
};

}  // namespace sle

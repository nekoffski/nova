#pragma once

#include <unordered_map>

#include <starlight/ui/UI.hh>
#include <starlight/ui/widgets/Image.hh>

#include "Resources.hh"
#include "resources/MaterialUI.hh"
#include "resources/TextureUI.hh"

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

    MaterialUI m_materialUI;
    TextureUI m_textureUI;
};

}  // namespace sle

#pragma once

#include <unordered_map>

#include <starlight/ui/UI.hh>
#include <starlight/ui/widgets/Image.hh>

#include "Resources.hh"

namespace sle {

class ResourcesView {
    struct Data {
        std::unordered_map<sl::u64, std::unique_ptr<sl::ui::ImageHandle>> textures;
    };

public:
    explicit ResourcesView(Resources& resources);

    void render();

private:
    void renderTexturesTab();
    void renderMeshesTab();
    void renderMaterialsTab();

    sl::ui::ImageHandle* getImageHandle(sl::Texture* texture);

    Resources& m_resources;
    sl::ui::TabMenu m_tabMenu;
    Data m_data;
};

}  // namespace sle

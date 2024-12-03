#pragma once

#include <starlight/ui/UI.hh>

namespace sle {

class SceneView {
public:
    explicit SceneView();

    void render();

private:
    sl::ui::TabMenu m_tab;
};

}  // namespace sle

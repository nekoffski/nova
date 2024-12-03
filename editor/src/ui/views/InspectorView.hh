#pragma once

#include <starlight/ui/UI.hh>

namespace sle {

class InspectorView {
public:
    explicit InspectorView();

    void render();

private:
    sl::ui::TabMenu m_tab;
};

}  // namespace sle

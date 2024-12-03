#include "InspectorView.hh"

namespace sle {

InspectorView::InspectorView() : m_tab("Inspector") {
    m_tab
      .addTab(
        ICON_FA_CUBES "  Entity",
        [&]() {

        }
      )
      .addTab(
        ICON_FA_IMAGE "  Resource",
        [&]() {

        }
      )
      .addTab(ICON_FA_EYE "  Renderer", [&]() {

      });
}

void InspectorView::render() { m_tab.render(); }

}  // namespace sle

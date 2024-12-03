#include "SceneView.hh"

namespace sle {

SceneView::SceneView() : m_tab("Scene") {
    m_tab
      .addTab(
        ICON_FA_CODE_BRANCH "  Entities Tree",
        [&]() {

        }
      )
      .addTab(
        ICON_FA_CLOUD "  Skybox",
        [&]() {

        }
      )
      .addTab(ICON_FA_CAMERA "  Camera", [&]() {

      });
}

void SceneView::render() { m_tab.render(); }

}  // namespace sle

#include "InspectorView.hh"

#include <starlight/core/event/EventProxy.hh>
#include <starlight/ui/UI.hh>

#include "Events.hh"

namespace sle {

InspectorView::InspectorView(sl::RenderGraph* renderGraph
) : m_tabMenu("Inspector"), m_rendererTab(renderGraph) {
    m_tabMenu.addTab(ICON_FA_CUBES "  Entity", [&]() { m_entityTab.render(); })
      .addTab(ICON_FA_IMAGE "  Resource", [&]() { m_resourceTab.render(); })
      .addTab(ICON_FA_EYE "  Renderer", [&]() { m_rendererTab.render(); });
}

void InspectorView::render() { m_tabMenu.render(); }

InspectorView::EntityTab::EntityTab() :
    m_eventSentinel(sl::EventProxy::get()), m_selectedEntity(nullptr) {
    m_eventSentinel.pushHandler<events::EntitySelected>([&](auto event) {
        m_selectedEntity = event.entity;
        return sl::EventChainBehaviour::propagate;
    });
}

void InspectorView::EntityTab::render() {
    if (m_selectedEntity == nullptr) {
        sl::ui::text("No entity selected");
        return;
    }

    const auto name = m_selectedEntity->getName();

    sl::ui::namedScope(name, [&]() {
        sl::ui::text(name);
        sl::ui::separator();

        static int selectedComponentIndex = 0;

        static std::vector<const char*> components = {
            "MeshTree",
            "PointLight",
        };

        ImGui::Combo(
          "##combo2", &selectedComponentIndex, components.data(), components.size()
        );

        sl::ui::sameLine();

        if (sl::ui::button("Add Component", sl::ui::parentWidth)) {
            EDITOR_LOG_DEBUG("Add component clicked");
        }

        sl::ui::separator();
    });
}

void InspectorView::ResourceTab::render() {}

InspectorView::RendererTab::RendererTab(sl::RenderGraph* renderGraph
) : m_renderGraph(renderGraph) {}

void InspectorView::RendererTab::setRenderGraph(sl::RenderGraph& renderGraph) {
    m_renderGraph = &renderGraph;
}

void InspectorView::setRenderGraph(sl::RenderGraph& renderGraph) {
    m_rendererTab.setRenderGraph(renderGraph);
}

void InspectorView::RendererTab::render() {
    sl::ui::namedScope("inspector-view-renderer-tab", [&]() {
        sl::ui::text("Render graph");

        for (auto& [view, renderPass] : m_renderGraph->getNodes()) {
            sl::ui::treeNode(
              std::string{ view->getName() },
              [&]() {
                  auto properties = renderPass->getProperties();
                  sl::ui::text(
                    "Rect: {}/{} - {}/{}", properties.rect.offset.x,
                    properties.rect.offset.y, properties.rect.size.w,
                    properties.rect.size.h
                  );
                  sl::ui::text(
                    "Render targets: {}", properties.renderTargets.size()
                  );
                  sl::ui::text(
                    "Depth included: {}", properties.includeDepthAttachment
                  );

                  auto colorPtr = sl::math::value_ptr(properties.clearColor);
                  if (ImGui::ColorEdit4("Clear Color", colorPtr))
                      renderPass->setClearColor(properties.clearColor);
              },
              ImGuiTreeNodeFlags_DefaultOpen
            );
        }
    });
}

}  // namespace sle

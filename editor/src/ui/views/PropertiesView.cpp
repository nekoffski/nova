#include "PropertiesView.hh"

#include <starlight/core/event/EventProxy.hh>

#include "Events.hh"

namespace sle {

#define ADD_COMPONENT(Component, ...)                                \
    if (m_data.selectedEntity->hasComponent<Component>()) {          \
        EDITOR_LOG_WARN("Component already added, skipping...");     \
    } else {                                                         \
        m_data.selectedEntity->addComponent<Component>(__VA_ARGS__); \
    }

PropertiesView::PropertiesView(sl::RenderGraph* renderGraph
) : m_tabMenu("Properties"), m_resourceTab(), m_rendererTab(renderGraph) {
    m_tabMenu.addTab(ICON_FA_CUBES "  Entity", [&]() { m_entityTab.render(); })
      .addTab(ICON_FA_IMAGE "  Resource", [&]() { m_resourceTab.render(); })
      .addTab(ICON_FA_EYE "  Renderer", [&]() { m_rendererTab.render(); });
}

void PropertiesView::render() { m_tabMenu.render(); }

PropertiesView::EntityTab::EntityTab() : m_eventSentinel(sl::EventProxy::get()) {
    m_eventSentinel
      .add<events::SetComponentUICallback>([&](auto& event) {
          m_componentCallback = event.callback;
          return sl::EventChainBehaviour::stop;
      })
      .add<events::EntitySelected>([&](auto& event) {
          m_data.selectedEntity = event.entity;
          m_data.nameBuffer     = event.entity->name;

          if (event.clearComponentCallback) m_componentCallback.reset();

          return sl::EventChainBehaviour::propagate;
      });
}

void PropertiesView::EntityTab::render() {
    if (not m_data.selectedEntity) {
        sl::ui::text("No entity selected");
        return;
    }

    renderEntityUI();
    sl::ui::separator();

    if (not m_componentCallback) {
        sl::ui::text("No component  selected");
        return;
    }
    std::invoke(m_componentCallback.value());
}

void PropertiesView::EntityTab::renderEntityUI() {
    static std::vector<const char*> componentNames = {
        "MeshComposite", "PointLight", "DirectionalLight"
    };

    sl::ui::namedScope(m_data.selectedEntity->name, [&]() {
        if (ImGui::InputText(
              "##", &m_data.nameBuffer, ImGuiInputTextFlags_EnterReturnsTrue
            )) {
            EDITOR_LOG_DEBUG("Entity name changed to: {}", m_data.nameBuffer);
            m_data.selectedEntity->name = m_data.nameBuffer;
        }
        sl::ui::separator();

        ImGui::Combo(
          "##combo2", &m_data.selectedComponentIndex, componentNames.data(),
          componentNames.size()
        );

        sl::ui::sameLine();

        if (sl::ui::button("Add Component", sl::ui::parentWidth)) {
            EDITOR_LOG_DEBUG(
              "Add component clicked: {}/{}", m_data.selectedEntity->name,
              m_data.selectedComponentIndex
            );

            if (m_data.selectedComponentIndex == 0) {
                ADD_COMPONENT(
                  sl::MeshComposite, sl::MeshFactory::get().getCube(),
                  sl::MaterialFactory::get().getDefault()
                );
            } else if (m_data.selectedComponentIndex == 1) {
                ADD_COMPONENT(sl::PointLight);
            } else if (m_data.selectedComponentIndex == 2) {
                ADD_COMPONENT(sl::DirectionalLight);
            }
        }

        if (sl::ui::button("Remove Entity", sl::ui::parentWidth)) {
        }
    });
}

PropertiesView::ResourceTab::ResourceTab() : m_eventSentinel(sl::EventProxy::get()) {
    m_eventSentinel.add<events::SetResourceUICallback>([&](auto& event) {
        m_resourceCallback = event.callback;
        return sl::EventChainBehaviour::stop;
    });
}

void PropertiesView::ResourceTab::render() {
    if (not m_resourceCallback) {
        sl::ui::text("No resource selected");
        return;
    }
    std::invoke(m_resourceCallback.value());
}

PropertiesView::RendererTab::RendererTab(sl::RenderGraph* renderGraph
) : m_renderGraph(renderGraph) {}

void PropertiesView::RendererTab::setRenderGraph(sl::RenderGraph& renderGraph) {
    m_renderGraph = &renderGraph;
}

void PropertiesView::setRenderGraph(sl::RenderGraph& renderGraph) {
    m_rendererTab.setRenderGraph(renderGraph);
}

void PropertiesView::RendererTab::render() {
    sl::ui::namedScope("inspector-view-renderer-tab", [&]() {
        sl::ui::text("Render graph");

        m_renderGraph->traverse(
          [&](sl::u32 index, bool active, auto& view, auto& renderPass) {
              const auto& name = view.name;
              sl::ui::treeNode(
                name,
                [&]() {
                    sl::ui::sameLine();
                    sl::ui::namedScope(name, [&]() {
                        if (sl::ui::checkbox("Active", active))
                            m_renderGraph->toggleView(index);
                    });

                    auto properties = renderPass.getProperties();
                    sl::ui::text(
                      "Rect: {}/{} - {}/{}", properties.rect.offset.x,
                      properties.rect.offset.y, properties.rect.size.w,
                      properties.rect.size.h
                    );
                    sl::ui::text(
                      "Render targets: {}", properties.renderTargets.size()
                    );

                    auto colorPtr = sl::math::value_ptr(properties.clearColor);
                    if (ImGui::ColorEdit4("Clear Color", colorPtr))
                        renderPass.setClearColor(properties.clearColor);
                },
                ImGuiTreeNodeFlags_DefaultOpen
              );
          }
        );
    });
}

}  // namespace sle

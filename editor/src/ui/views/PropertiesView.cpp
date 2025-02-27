#include "PropertiesView.hh"

#include <starlight/event/EventProxy.hh>

#include "Events.hh"

namespace sle {

#define ADD_COMPONENT(Component, ...)                                \
    if (m_data.selectedEntity->hasComponent<Component>()) {          \
        editorWriteWarn("Component already added, skipping...");     \
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
      .add<events::SetComponentUICallback>([&](auto& event, auto&& handled) {
          m_componentCallback = event.callback;
          handled();
      })
      .add<events::EntitySelected>([&](auto& event) {
          m_data.selectedEntity = event.entity;
          m_data.nameBuffer     = event.entity->name;
          if (event.clearComponentCallback) m_componentCallback.reset();
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
            editorWriteDebug("Entity name changed to: {}", m_data.nameBuffer);
            m_data.selectedEntity->name = m_data.nameBuffer;
        }
        sl::ui::separator();

        ImGui::Combo(
          "##combo2", &m_data.selectedComponentIndex, componentNames.data(),
          componentNames.size()
        );

        sl::ui::sameLine();

        if (sl::ui::button("Add Component", sl::ui::parentWidth)) {
            editorWriteDebug(
              "Add component clicked: {}/{}", m_data.selectedEntity->name,
              m_data.selectedComponentIndex
            );

            if (m_data.selectedComponentIndex == 0) {
                // ADD_COMPONENT(
                //   sl::MeshComposite, sl::MeshFactory::get().getCube(),
                //   sl::MaterialFactory::get().getDefault()
                // );
            } else if (m_data.selectedComponentIndex == 1) {
                // ADD_COMPONENT(sl::PointLight);
            } else if (m_data.selectedComponentIndex == 2) {
                // ADD_COMPONENT(sl::DirectionalLight);
            }
        }

        if (sl::ui::button("Remove Entity", sl::ui::parentWidth)) {
        }
    });
}

PropertiesView::ResourceTab::ResourceTab() : m_eventSentinel(sl::EventProxy::get()) {
    m_eventSentinel.add<events::SetResourceUICallback>(
      [&](auto& event, auto&& handled) {
          m_resourceCallback = event.callback;
          handled();
      }
    );
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
        sl::ui::text(ICON_FA_NETWORK_WIRED "   Render graph");
        sl::ui::separator();

        bool changed = false;

        m_renderGraph->forEach([&](auto& active, auto& renderPass) {
            if (renderPass.name != "UIRenderPass") {  // TODO: find better way
                sl::ui::namedScope(renderPass.name, [&]() {
                    if (sl::ui::checkbox("##Active", active)) changed = true;
                    sl::ui::sameLine();
                    sl::ui::text(renderPass.name);
                });
            }
        });

        if (changed) {
            // sl::TaskQueue::get().callPostFrame([&]() {
            //     m_renderGraph->rebuildChain();
            // });
        }
    });
}

}  // namespace sle

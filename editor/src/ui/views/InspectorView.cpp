#include "InspectorView.hh"

#include <starlight/core/event/EventProxy.hh>

#include "Events.hh"

namespace sle {

#define ADD_COMPONENT(Component, ...)                            \
    if (m_selectedEntity->hasComponent<Component>()) {           \
        EDITOR_LOG_WARN("Component already added, skipping..."); \
    } else {                                                     \
        m_selectedEntity->addComponent<Component>(__VA_ARGS__);  \
    }

#define RENDER_COMPONENT(Component)                                          \
    if (m_selectedEntity->hasComponent<Component>()) {                       \
        renderComponent(m_selectedEntity->getComponent<Component>().data()); \
    }

InspectorView::InspectorView(Resources& resources, sl::RenderGraph* renderGraph) :
    m_tabMenu("Inspector"), m_entityTab(resources), m_resourceTab(resources),
    m_rendererTab(renderGraph) {
    m_tabMenu.addTab(ICON_FA_CUBES "  Entity", [&]() { m_entityTab.render(); })
      .addTab(ICON_FA_IMAGE "  Resource", [&]() { m_resourceTab.render(); })
      .addTab(ICON_FA_EYE "  Renderer", [&]() { m_rendererTab.render(); });
}

void InspectorView::render() { m_tabMenu.render(); }

std::vector<const char*> componentNames = { "MeshComposite", "PointLight" };

InspectorView::EntityTab::EntityTab(Resources& resources) :
    m_resources(resources), m_eventSentinel(sl::EventProxy::get()),
    m_selectedEntity(nullptr), m_selectedComponentIndex(0) {
    m_eventSentinel.pushHandler<events::EntitySelected>([&](auto event) {
        m_selectedEntity        = event.entity;
        m_data.entityNameBuffer = m_selectedEntity->name;
        return sl::EventChainBehaviour::propagate;
    });
}

static std::string buffer;

void InspectorView::EntityTab::render() {
    if (m_selectedEntity == nullptr) {
        sl::ui::text("No entity selected");
        return;
    }

    auto& name = m_selectedEntity->name;

    sl::ui::namedScope(name, [&]() {
        if (ImGui::InputText(
              "##", &m_data.entityNameBuffer, ImGuiInputTextFlags_EnterReturnsTrue
            )) {
            EDITOR_LOG_DEBUG("Entity name changed to: {}", m_data.entityNameBuffer);
            name = m_data.entityNameBuffer;
        }
        sl::ui::separator();

        ImGui::Combo(
          "##combo2", &m_selectedComponentIndex, componentNames.data(),
          componentNames.size()
        );

        sl::ui::sameLine();

        if (sl::ui::button("Add Component", sl::ui::parentWidth)) {
            EDITOR_LOG_DEBUG(
              "Add component clicked: {}/{}", name, m_selectedComponentIndex
            );
            addComponent();
        }

        sl::ui::separator();
        sl::ui::namedScope(fmt::format("{}_components", name), [&]() {
            renderComponents();
        });
    });
}

void InspectorView::EntityTab::addComponent() {
    if (m_selectedComponentIndex == 0) {
        ADD_COMPONENT(
          sl::MeshComposite, sl::Mesh::getCube(), sl::Material::getDefault()
        );
    } else if (m_selectedComponentIndex == 1) {
    }
}

void InspectorView::EntityTab::renderComponents() {
    RENDER_COMPONENT(sl::MeshComposite);
}

void InspectorView::EntityTab::renderComponent(sl::MeshComposite& component) {
    auto renderInstance = [&](auto& instance, auto i) {
        sl::ui::treeNode(
          fmt::format("Instance_{}", i),
          [&]() {
              auto position = instance.getPosition();
              if (sl::ui::slider("Position", position, { -10.0f, 10.0f, 0.01f })) {
                  instance.setPosition(position);
              }
              auto scale = instance.getScale();
              if (sl::ui::slider("Scale", scale, { -5.0f, 5.0f, 0.01f })) {
                  instance.setScale(scale);
              }
          },
          ImGuiTreeNodeFlags_DefaultOpen
        );
    };

    sl::ui::treeNode(
      ICON_FA_PLANE "  MeshComposite",
      [&]() {
          component.traverse([&](auto& node) {
              sl::ui::treeNode(
                node.name,
                [&]() {
                    auto meshName = node.mesh.getName();

                    sl::ui::text("Mesh: ");
                    if (ImGui::BeginCombo("##mesh-combo", meshName.c_str())) {
                        for (auto& mesh : m_resources.meshes) {
                            bool selected = mesh.getName() == meshName;
                            if (ImGui::Selectable(mesh.getName().c_str(), selected))
                                node.mesh = mesh;
                            if (selected) ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }

                    auto materialName = node.material.getName();
                    sl::ui::text("Material: ");
                    if (ImGui::BeginCombo(
                          "##material-combo", materialName.c_str()
                        )) {
                        for (auto& material : m_resources.materials) {
                            bool selected = material.getName() == materialName;
                            if (ImGui::Selectable(
                                  material.getName().c_str(), selected
                                )) {
                                node.material = material;
                            }
                            if (selected) ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }

                    if (sl::ui::button("Add Instance")) node.addInstance();

                    auto instances = node.getInstances();
                    for (sl::u64 i = 0; i < instances.size(); ++i) {
                        renderInstance(instances[i], i);
                    }
                },
                ImGuiTreeNodeFlags_DefaultOpen
              );
          });
      },
      ImGuiTreeNodeFlags_DefaultOpen
    );
    sl::ui::separator();
}

InspectorView::ResourceTab::ResourceTab(Resources& resources
) : m_resources(resources) {}

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
                    sl::ui::text(
                      "Depth included: {}", properties.includeDepthAttachment
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

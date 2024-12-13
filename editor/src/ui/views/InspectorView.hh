#pragma once

#include <starlight/ui/UI.hh>
#include <starlight/scene/ecs/Entity.hh>
#include <starlight/scene/Scene.hh>
#include <starlight/core/event/EventHandlerSentinel.hh>
#include <starlight/renderer/RenderGraph.hh>

#include <starlight/renderer/MeshComposite.hh>
#include <starlight/renderer/light/PointLight.hh>
#include <starlight/renderer/light/DirectionalLight.hh>

#include "Console.hh"
#include "Resources.hh"

namespace sle {

class InspectorView {
    struct Data {
        std::string entityNameBuffer;
        sl::i32 selectedComponentIndex = 0u;
    };

    class EntityTab {
    public:
        explicit EntityTab(Resources& resources);

        void render();

    private:
        void addComponent();
        void renderComponents();

        // components
        void renderComponent(sl::MeshComposite& component);
        void renderComponent(sl::PointLight& light);
        void renderComponent(sl::DirectionalLight& light);

        Resources& m_resources;
        sl::EventHandlerSentinel m_eventSentinel;
        sl::Entity* m_selectedEntity;

        Data m_data;
    };

    class ResourceTab {
    public:
        explicit ResourceTab(Resources& resources);
        void render();

    private:
        Resources& m_resources;
    };

    class RendererTab {
    public:
        explicit RendererTab(sl::RenderGraph* renderGraph);

        void render();
        void setRenderGraph(sl::RenderGraph& renderGraph);

    private:
        sl::RenderGraph* m_renderGraph;
    };

public:
    explicit InspectorView(Resources& resources, sl::RenderGraph* renderGraph);

    void render();
    void setRenderGraph(sl::RenderGraph& renderGraph);

private:
    sl::ui::TabMenu m_tabMenu;

    EntityTab m_entityTab;
    ResourceTab m_resourceTab;
    RendererTab m_rendererTab;
};

}  // namespace sle

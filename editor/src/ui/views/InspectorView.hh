#pragma once

#include <starlight/ui/UI.hh>
#include <starlight/scene/Entity.hh>
#include <starlight/scene/Scene.hh>
#include <starlight/core/event/EventHandlerSentinel.hh>
#include <starlight/renderer/RenderGraph.hh>

#include <starlight/renderer/MeshComposite.hh>

#include "Console.hh"

namespace sle {

class InspectorView {
    class EntityTab {
    public:
        explicit EntityTab();

        void render();

    private:
        void addComponent();
        void renderComponents();

        // components
        void renderComponent(sl::MeshComposite& component);

        sl::EventHandlerSentinel m_eventSentinel;
        sl::Entity* m_selectedEntity;

        sl::i32 m_selectedComponentIndex;
    };

    class ResourceTab {
    public:
        void render();
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
    explicit InspectorView(sl::RenderGraph* renderGraph);

    void render();
    void setRenderGraph(sl::RenderGraph& renderGraph);

private:
    sl::ui::TabMenu m_tabMenu;

    EntityTab m_entityTab;
    ResourceTab m_resourceTab;
    RendererTab m_rendererTab;
};

}  // namespace sle

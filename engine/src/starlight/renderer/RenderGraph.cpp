#include "RenderGraph.hh"

#include "starlight/core/event/WindowResized.hh"

namespace sl {

RenderGraph::Builder::Builder(
  RendererBackend& renderer, EventProxy& eventProxy, const Vec2<u32>& viewportSize
) : m_renderer(renderer), m_eventProxy(eventProxy), m_viewportSize(viewportSize) {}

namespace {

RenderPass::ChainFlags getChainFlags(u32 index, u32 count) {
    auto chainFlags = RenderPass::ChainFlags::none;

    if (index != 0) chainFlags |= RenderPass::ChainFlags::hasPrevious;
    if (index != count - 1) chainFlags |= RenderPass::ChainFlags::hasNext;

    return chainFlags;
}

}  // namespace

OwningPtr<RenderGraph> RenderGraph::Builder::build() && {
    auto renderGraph = createOwningPtr<RenderGraph>(m_eventProxy);
    auto& nodes      = renderGraph->getNodes();

    const auto viewCount = m_renderViews.size();
    nodes.reserve(viewCount);

    for (u32 i = 0; i < viewCount; ++i) {
        auto& view            = m_renderViews[i];
        const auto chainFlags = getChainFlags(i, viewCount);

        // TODO: kind of a cyclic dependency, try to decouple
        const auto props = view->getRenderPassProperties(m_renderer, chainFlags);
        auto renderPass  = RenderPass::create(m_renderer, props, chainFlags);
        view->init(m_renderer, *renderPass);

        nodes.emplace_back(i, std::move(view), std::move(renderPass));
    }

    return renderGraph;
}

RenderGraph::RenderGraph(EventProxy& eventProxy) : m_eventSentinel(eventProxy) {
    m_eventSentinel.add<WindowResized>([&](const auto& event) {
        onViewportResize(event.size);
        return sl::EventChainBehaviour::propagate;
    });
}

void RenderGraph::onViewportResize(const Vec2<u32>& viewport) {
    for (auto& node : m_nodes) {
        node.renderPass->regenerateRenderTargets(viewport);
        node.renderPass->setRectSize(viewport);
    }
}

void RenderGraph::disableView(sl::u32 index) {
    ASSERT(index < m_nodes.size(), "Render view index out of bounds");
    m_nodes[index].isActive = false;
    // FIXME: rebuild
}

void RenderGraph::toggleView(sl::u32 index) {
    ASSERT(index < m_nodes.size(), "Render view index out of bounds");
    auto& node    = m_nodes[index];
    node.isActive = !node.isActive;
    // FIXME: rebuild
}

void RenderGraph::enableView(sl::u32 index) {
    ASSERT(index < m_nodes.size(), "Render view index out of bounds");
    m_nodes[index].isActive = true;
    // FIXME: rebuild
}

std::vector<RenderGraph::Node>& RenderGraph::getNodes() { return m_nodes; }

}  // namespace sl

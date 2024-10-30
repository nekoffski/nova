#include "RenderGraph.hh"

namespace sl {

RenderGraph::Builder::Builder(
  RendererBackend& renderer, const Vec2<u32>& viewportSize
) :
    m_renderer(renderer), m_viewportSize(viewportSize),
    m_renderGraph(createOwningPtr<RenderGraph>()) {}

OwningPtr<RenderGraph> RenderGraph::Builder::build() && {
    auto& views          = m_renderGraph->getViews();
    const auto viewCount = views.size();

    RenderView::InitProperties initProperties{ m_viewportSize, false, false };

    for (u32 i = 0; i < viewCount; ++i) {
        initProperties.hasPreviousView = (i != 0);
        initProperties.hasNextView     = (i != viewCount - 1);
        views[i]->init(m_renderer, initProperties);
    }

    return std::move(m_renderGraph);
}

RenderGraph::Views& RenderGraph::getViews() { return m_views; }

const RenderGraph::Views& RenderGraph::getViews() const { return m_views; }

}  // namespace sl

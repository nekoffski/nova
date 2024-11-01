#include "RenderGraph.hh"

namespace sl {

RenderGraph::Builder::Builder(
  RendererBackend& renderer, const Vec2<u32>& viewportSize
) :
    m_renderer(renderer), m_viewportSize(viewportSize),
    m_renderGraph(createOwningPtr<RenderGraph>()) {}

namespace {

RenderPass::ChainFlags getChainFlags(u32 index, u32 count) {
    auto chainFlags = RenderPass::ChainFlags::none;

    if (index != 0) chainFlags |= RenderPass::ChainFlags::hasPrevious;
    if (index != count - 1) chainFlags |= RenderPass::ChainFlags::hasNext;

    return chainFlags;
}

}  // namespace

OwningPtr<RenderGraph> RenderGraph::Builder::build() && {
    auto& views        = m_renderGraph->getViews();
    auto& renderPasses = m_renderGraph->getRenderPasses();
    auto& nodes        = m_renderGraph->getNodes();

    const auto viewCount = views.size();

    renderPasses.reserve(viewCount);
    nodes.reserve(viewCount);

    for (u32 i = 0; i < viewCount; ++i) {
        auto view             = views[i].get();
        const auto chainFlags = getChainFlags(i, viewCount);
        const auto renderPassProps =
          view->getRenderPassProperties(m_renderer, chainFlags);

        renderPasses.push_back(
          RenderPass::create(m_renderer, renderPassProps, chainFlags)
        );

        auto renderPass = renderPasses.back().get();

        view->init(*renderPass);
        nodes.emplace_back(view, renderPass);
    }

    return std::move(m_renderGraph);
}

std::vector<OwningPtr<RenderView>>& RenderGraph::getViews() { return m_views; }

std::vector<OwningPtr<RenderPass>>& RenderGraph::getRenderPasses() {
    return m_renderPasses;
}

RenderGraph::Nodes& RenderGraph::getNodes() { return m_nodes; }

const RenderGraph::Nodes& RenderGraph::getNodes() const { return m_nodes; }

void RenderGraph::onViewportResize(const Vec2<u32>& viewport) {
    for (auto& renderPass : m_renderPasses) {
        renderPass->regenerateRenderTargets(viewport);
        renderPass->setRectSize(viewport);
    }
}

}  // namespace sl

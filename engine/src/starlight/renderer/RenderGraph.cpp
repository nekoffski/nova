#include "RenderGraph.hh"

#include <ranges>

#include "starlight/core/event/WindowResized.hh"

namespace sl {

RenderGraph::RenderGraph(Renderer& renderer) : m_renderer(renderer) {}

void RenderGraph::render(RenderPacket& renderPacket) {
    m_renderer.renderFrame([&](CommandBuffer& commandBuffer, u32 imageIndex) {
        for (auto& renderPass : m_activeRenderPasses)
            renderPass->run(renderPacket, commandBuffer, imageIndex);
    });
}

void RenderGraph::rebuildChain() {
    std::vector<RenderPassBase*> activePasses;
    activePasses.reserve(m_nodes.size());

    for (auto& [renderPass, active] : m_nodes)
        if (active) activePasses.push_back(renderPass.get());

    const auto n = activePasses.size();

    for (u64 i = 0; i < n; ++i) {
        bool hasPreviousPass = (i != 0);
        bool hasNextPass     = (i != (n - 1));
        activePasses[i]->init(hasPreviousPass, hasNextPass);
    }

    std::swap(activePasses, m_activeRenderPasses);
}

}  // namespace sl

#include "RenderGraph.hh"

#include <ranges>

#include "starlight/window/Events.hh"

namespace sl {

RenderGraph::RenderGraph(Renderer& renderer
) : m_renderer(renderer), m_eventSentinel(EventProxy::get()) {
    m_eventSentinel.add<WindowResized>([&]([[maybe_unused]] auto&) {
        onWindowResize();
    });
}

void RenderGraph::render(RenderPacket& renderPacket) {
    m_renderer.renderFrame(
      [&](CommandBuffer& commandBuffer, u32 imageIndex, u64 frameNumber) {
          for (auto& renderPass : m_activeRenderPasses)
              renderPass->run(renderPacket, commandBuffer, imageIndex, frameNumber);
      }
    );
}

void RenderGraph::onWindowResize() { rebuildChain(); }

void RenderGraph::rebuildChain() {
    log::debug("Rebuilding render graph chain");
    std::vector<RenderPassBase*> activePasses;
    activePasses.reserve(m_nodes.size());

    for (auto& [renderPass, active] : m_nodes) {
        log::debug("{} - {}", renderPass->name, active ? "ACTIVE" : "INACTIVE");
        if (active) activePasses.push_back(renderPass.get());
    }

    const auto n = activePasses.size();

    for (u64 i = 0; i < n; ++i) {
        bool hasPreviousPass = (i != 0);
        bool hasNextPass     = (i != (n - 1));
        activePasses[i]->init(hasPreviousPass, hasNextPass);
    }

    std::swap(activePasses, m_activeRenderPasses);
}

}  // namespace sl

#pragma once

#include <vector>

#include "starlight/core/memory/Memory.hh"
#include "starlight/core/event/EventProxy.hh"
#include "starlight/core/math/Core.hh"
#include "gpu/RendererBackend.hh"
#include "RenderPass.hh"
#include "Renderer.hh"
#include "RenderPacket.hh"

namespace sl {

class RenderGraph {
    struct Node {
        OwningPtr<RenderPassBase> renderPass;
        bool active = true;
    };

public:
    explicit RenderGraph(Renderer& renderer);

    template <typename T, typename... Args>
    requires(std::is_base_of_v<RenderPassBase, T> && std::constructible_from<T, Renderer&, Args...>)
    RenderPassBase* addRenderPass(Args&&... args) {
        m_nodes.emplace_back(
          createOwningPtr<T>(m_renderer, std::forward<Args>(args)...)
        );
        rebuildChain();
        return m_nodes.back().renderPass.get();
    }

    void render(RenderPacket& renderPacket);

private:
    void onWindowResize();

    void rebuildChain();

    Renderer& m_renderer;
    EventHandlerSentinel m_eventSentinel;

    std::vector<Node> m_nodes;
    std::vector<RenderPassBase*> m_activeRenderPasses;
};

}  // namespace sl

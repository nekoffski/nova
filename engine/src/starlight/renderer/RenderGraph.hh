#pragma once

#include <vector>

#include "starlight/core/memory/Memory.hh"
#include "starlight/event/EventProxy.hh"
#include "starlight/core/math/Core.hh"
#include "starlight/core/Concepts.hh"
#include "RenderPass.hh"
#include "Renderer.hh"
#include "RenderPacket.hh"

namespace sl {

class RenderGraph {
public:
    struct Node {
        UniquePointer<RenderPassBase> renderPass;
        bool active = true;
    };

    explicit RenderGraph(Renderer& renderer);

    template <typename T, typename... Args>
    requires(std::is_base_of_v<RenderPassBase, T> && std::constructible_from<T, Renderer&, Args...>)
    RenderPassBase* addPass(Args&&... args) {
        m_nodes.emplace_back(
          UniquePointer<T>::create(m_renderer, std::forward<Args>(args)...)
        );
        rebuildChain();
        return m_nodes.back().renderPass.get();
    }

    template <typename Callback>
    requires Callable<Callback, void, bool&, RenderPassBase&>
    void forEach(Callback&& callback) {
        for (auto& node : m_nodes) callback(node.active, *node.renderPass);
    }

    void render(RenderPacket& renderPacket);
    void rebuildChain();

private:
    void onWindowResize();

    Renderer& m_renderer;
    EventHandlerSentinel m_eventSentinel;

    std::vector<Node> m_nodes;
    std::vector<RenderPassBase*> m_activeRenderPasses;
};

}  // namespace sl

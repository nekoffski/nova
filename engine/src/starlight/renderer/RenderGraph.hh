#pragma once

#include <vector>

#include "starlight/core/memory/Memory.hh"
#include "starlight/core/event/EventProxy.hh"
#include "starlight/core/math/Core.hh"
#include "gpu/RendererBackend.hh"
#include "views/RenderView.hh"
#include "gpu/RenderPass.hh"
#include "Renderer.hh"
#include "RenderPacket.hh"

namespace sl {

class RenderGraph {
    struct Node {
        OwningPtr<RenderPass> renderPass;
        bool active = true;
    };

public:
    explicit RenderGraph(Renderer& renderer);

    template <typename T, typename... Args>
    requires(std::is_base_of_v<RenderPass, T> && std::constructible_from<T, Renderer&, Args...>)
    RenderPass* addRenderPass(Args&&... args) {
        m_nodes.emplace_back(
          createOwningPtr<T>(m_renderer, std::forward<Args>(args)...)
        );
        rebuildChain();
        return m_nodes.back().renderPass.get();
    }

    void render(RenderPacket& renderPacket);

private:
    void rebuildChain();

    Renderer& m_renderer;
    std::vector<Node> m_nodes;
    std::vector<RenderPass*> m_activeRenderPasses;
};

}  // namespace sl

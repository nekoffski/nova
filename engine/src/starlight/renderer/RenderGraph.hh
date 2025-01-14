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

namespace v2 {

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
};
}  // namespace v2

class RenderGraph {
    friend class Builder;

    struct Node {
        sl::u32 index;
        OwningPtr<RenderView> view;
        OwningPtr<RenderPass> renderPass;
        bool isActive = true;
    };

public:
    enum class TraverseMode : sl::u8 { activeOnly, all };

    class Builder {
    public:
        explicit Builder(
          RendererBackend& renderer, EventProxy& eventProxy,
          const Vec2<u32>& viewportSize
        );

        OwningPtr<RenderGraph> build() &&;

        template <typename T, typename... Args>
        Builder&& addView(Args&&... args) && {
            m_renderViews.push_back(createOwningPtr<T>(std::forward<Args>(args)...));
            return std::move(*this);
        }

    private:
        RendererBackend& m_renderer;
        EventProxy& m_eventProxy;
        const Vec2<u32>& m_viewportSize;
        std::vector<OwningPtr<RenderView>> m_renderViews;
    };

    explicit RenderGraph(EventProxy& eventProxy);

    template <typename C>
    requires Callable<C, void, sl::u32, bool, RenderView&, RenderPass&>
    void traverse(C&& callback) {
        for (auto& node : m_nodes) {
            callback(node.index, node.isActive, *node.view, *node.renderPass);
        }
    }

    template <typename C>
    requires Callable<C, void, RenderView&, RenderPass&>
    void traverse(C&& callback, TraverseMode mode) {
        for (auto& node : m_nodes) {
            if (mode == TraverseMode::all || node.isActive)
                callback(*node.view, *node.renderPass);
        }
    }

    void enableView(sl::u32 index);
    void disableView(sl::u32 index);
    void toggleView(sl::u32 index);

private:
    void onViewportResize(const Vec2<u32>& viewport);

    EventHandlerSentinel m_eventSentinel;

    std::vector<Node>& getNodes();
    std::vector<Node> m_nodes;
};

}  // namespace sl

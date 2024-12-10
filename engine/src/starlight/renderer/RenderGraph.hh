#pragma once

#include <vector>

#include "starlight/core/memory/Memory.hh"
#include "starlight/core/math/Core.hh"
#include "gpu/RendererBackend.hh"
#include "views/RenderView.hh"

namespace sl {

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
        explicit Builder(RendererBackend& renderer, const Vec2<u32>& viewportSize);

        OwningPtr<RenderGraph> build() &&;

        template <typename T, typename... Args>
        Builder&& addView(Args&&... args) && {
            m_renderViews.push_back(createOwningPtr<T>(std::forward<Args>(args)...));
            return std::move(*this);
        }

    private:
        RendererBackend& m_renderer;
        const Vec2<u32>& m_viewportSize;
        std::vector<OwningPtr<RenderView>> m_renderViews;
    };

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

    void onViewportResize(const Vec2<u32>& viewport);

    void enableView(sl::u32 index);
    void disableView(sl::u32 index);
    void toggleView(sl::u32 index);

private:
    std::vector<Node>& getNodes();
    std::vector<Node> m_nodes;
};

}  // namespace sl

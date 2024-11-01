#pragma once

#include <vector>

#include "starlight/core/memory/Memory.hh"
#include "starlight/core/math/Core.hh"
#include "gpu/RendererBackend.hh"
#include "views/RenderView.hh"

namespace sl {

class RenderGraph {
    struct Node {
        RenderView* view;
        RenderPass* renderPass;
    };

    friend class Builder;

public:
    using Nodes = std::vector<Node>;

    class Builder {
    public:
        explicit Builder(RendererBackend& renderer, const Vec2<u32>& viewportSize);

        OwningPtr<RenderGraph> build() &&;

        template <typename T, typename... Args>
        Builder&& addView(Args&&... args) && {
            m_renderGraph->getViews().push_back(
              createOwningPtr<T>(std::forward<Args>(args)...)
            );
            return std::move(*this);
        }

    private:
        RendererBackend& m_renderer;
        const Vec2<u32>& m_viewportSize;

        OwningPtr<RenderGraph> m_renderGraph;
    };

    const Nodes& getNodes() const;
    Nodes& getNodes();

    void onViewportResize(const Vec2<u32>& viewport);

private:
    std::vector<OwningPtr<RenderView>>& getViews();
    std::vector<OwningPtr<RenderPass>>& getRenderPasses();

    Nodes m_nodes;

    std::vector<OwningPtr<RenderView>> m_views;
    std::vector<OwningPtr<RenderPass>> m_renderPasses;
};

}  // namespace sl

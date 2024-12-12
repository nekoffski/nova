#pragma once

#include <vector>

#include "starlight/core/Core.hh"
#include "starlight/core/math/Core.hh"
#include "starlight/core/utils/Enum.hh"
#include "starlight/core/utils/Id.hh"
#include "starlight/core/utils/Resource.hh"

#include "RenderTarget.hh"
#include "CommandBuffer.hh"

#include "fwd.hh"

namespace sl {

class RenderPass : public NonMovable, public Identificable<RenderTarget> {
public:
    enum class ClearFlags : unsigned char {
        none    = 0x0,
        color   = 0x1,
        depth   = 0x2,
        stencil = 0xc4
    };

    enum class ChainFlags : unsigned char {
        none        = 0x0,
        hasNext     = 0x1,
        hasPrevious = 0x2
    };

    enum class State : unsigned char {
        ready,
        recording,
        inRenderPass,
        recordingEnded,
        submitted,
        notAllocated
    };

    struct Properties {
        Rect2<u32> rect;
        Vec4<f32> clearColor;
        ClearFlags clearFlags;
        std::vector<RenderTarget> renderTargets;
        bool includeDepthAttachment;
    };

    static OwningPtr<RenderPass> create(
      RendererBackend& renderer, const Properties& props,
      ChainFlags chainFlags = ChainFlags::none
    );

    explicit RenderPass(const Properties& props);
    virtual ~RenderPass() = default;

    template <typename C>
    requires Callable<C, void, CommandBuffer&, u8>
    void run(CommandBuffer& commandBuffer, u8 attachmentIndex, C&& callback) {
        begin(commandBuffer, attachmentIndex);
        callback(commandBuffer, attachmentIndex);
        end(commandBuffer);
    }

    void setClearColor(const Vec4<f32>& color);
    void setRect(const Rect2<u32>& extent);
    void setRectSize(const Vec2<u32>& size);

    const Properties& getProperties() const;

    virtual void regenerateRenderTargets(const Vec2<u32>& viewportSize) = 0;

private:
    virtual void begin(CommandBuffer& commandBuffer, u8 attachmentIndex) = 0;
    virtual void end(CommandBuffer& commandBuffer)                       = 0;

protected:
    Properties m_props;
};

constexpr void enableBitOperations(RenderPass::ClearFlags);
constexpr void enableBitOperations(RenderPass::ChainFlags);

}  // namespace sl

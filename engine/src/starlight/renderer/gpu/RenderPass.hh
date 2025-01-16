#pragma once

#include <vector>
#include <optional>

#include "starlight/core/Core.hh"
#include "starlight/core/math/Core.hh"
#include "starlight/core/utils/Enum.hh"
#include "starlight/core/utils/Id.hh"
#include "starlight/core/utils/Resource.hh"
#include "starlight/renderer/RenderPacket.hh"

#include "Texture.hh"
#include "RenderTarget.hh"
#include "CommandBuffer.hh"

#include "fwd.hh"
#include "starlight/renderer/fwd.hh"

namespace sl {

class RenderPass : public NonMovable, public Identificable<RenderPass> {
public:
    class Impl : public NonMovable, public NonCopyable {
    public:
        virtual ~Impl() = default;

        template <typename Callback>
        requires Callable<Callback, void, CommandBuffer&, u8>
        void run(CommandBuffer& commandBuffer, u32 imageIndex, Callback&& callback) {
            begin(commandBuffer, imageIndex);
            callback(commandBuffer, imageIndex);
            end(commandBuffer);
        }

    private:
        virtual void begin(CommandBuffer& commandBuffer, u32 imageIndex) = 0;
        virtual void end(CommandBuffer& commandBuffer)                   = 0;
    };

    enum class ClearFlags : u8 {
        none    = 0x0,
        color   = 0x1,
        depth   = 0x2,
        stencil = 0x4
    };

    enum class Attachment : u8 { none = 0x0, swapchainColor = 0x1, depth = 0x2 };

    struct RenderTarget {
        Texture* colorAttachment = nullptr;
        Texture* depthAttachment = nullptr;
    };

    struct Properties {
        Rect2<u32> rect;
        Vec4<f32> clearColor;
        ClearFlags clearFlags;
        std::vector<RenderTarget> renderTargets;
        f32 depth   = 1.0f;
        u32 stencil = 0u;
    };

    explicit RenderPass(
      Renderer& renderer, const Vec2<f32>& viewportOffset = { 0.0f, 0.0f },
      std::optional<std::string> name = {}
    );

    void init(bool hasPreviousPass, bool hasNextPass);
    void run(RenderPacket& packet, CommandBuffer& commandBuffer, u32 imageIndex);

protected:
    Properties createDefaultProperties(
      Attachment attachments, ClearFlags clearFlags
    );

    virtual Rect2<u32> getViewport();
    virtual Properties createProperties(bool hasPreviousPass, bool hasNextPass) = 0;

private:
    virtual void render(
      RenderPacket& packet, CommandBuffer& commandBuffer, u32 imageIndex
    ) = 0;

    Renderer& m_renderer;
    Vec2<f32> m_viewportOffset;
    OwningPtr<Impl> m_renderPassImpl;
    // OwningPtr<Pipeline> m_pipeline;

public:
    const std::string name;
};

constexpr void enableBitOperations(RenderPass::ClearFlags);
constexpr void enableBitOperations(RenderPass::Attachment);

}  // namespace sl

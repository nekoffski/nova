#pragma once

#include "starlight/core/Core.hh"
#include "starlight/renderer/Core.hh"
#include "CommandBuffer.hh"
#include "Texture.hh"

namespace sl {

struct RenderTarget {
    Texture* colorAttachment = nullptr;
    Texture* depthAttachment = nullptr;
};

class RenderPassBackend : public NonMovable, public NonCopyable {
public:
    struct Properties {
        Rect2<u32> rect;
        Vec4<f32> clearColor;
        ClearFlags clearFlags;
        std::vector<RenderTarget> renderTargets;
        f32 depth   = 1.0f;
        u32 stencil = 0u;
    };

    virtual ~RenderPassBackend() = default;

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

}  // namespace sl

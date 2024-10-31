#pragma once

#include <array>
#include <span>
#include <functional>

#include "starlight/core/math/Core.hh"
#include "starlight/core/math/Vertex.hh"
#include "starlight/core/math/Vertex.hh"

#include "starlight/renderer/fwd.hh"
#include "starlight/renderer/Material.hh"
#include "starlight/renderer/gpu/Mesh.hh"
#include "starlight/renderer/gpu/Texture.hh"
#include "starlight/renderer/gpu/Shader.hh"
#include "starlight/renderer/gpu/CommandBuffer.hh"

namespace sl {

class RendererBackend : public NonCopyable, public NonMovable {
public:
    virtual ~RendererBackend() = default;

    template <typename C>
    requires Callable<C, void, CommandBuffer&, u8>
    u64 renderFrame(float deltaTime, C&& callback) {
        if (beginFrame(deltaTime)) {
            callback(getCommandBuffer(), getImageIndex());
            endFrame(deltaTime);
        }
        return getRenderedVertexCount();
    }

    virtual void setViewport(const Rect2<u32>& viewport) = 0;

    virtual u64 getRenderedVertexCount() const = 0;

    virtual bool beginFrame(float deltaTime) = 0;
    virtual bool endFrame(float deltaTime)   = 0;

    virtual void drawMesh(const Mesh& mesh) = 0;

    virtual void onViewportResize(const Vec2<u32>& viewportSize) = 0;

    virtual CommandBuffer& getCommandBuffer()  = 0;  // should be private
    virtual u32 getSwapchainImageCount() const = 0;
    // TODO: unify, either image or texture
    virtual u32 getImageIndex() const               = 0;  // should be private
    virtual Texture* getSwapchainTexture(u32 index) = 0;
    virtual Texture* getDepthTexture()              = 0;
};

}  // namespace sl

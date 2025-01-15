#pragma once

#include "starlight/core/Core.hh"
#include "starlight/core/Context.hh"
#include "starlight/core/memory/Memory.hh"

#include "Queue.hh"
#include "Sync.hh"
#include "Swapchain.hh"
#include "RenderPass.hh"
#include "Texture.hh"

namespace sl {

class Device : public NonCopyable, public NonMovable {
public:
    virtual ~Device() = default;

    static OwningPtr<Device> create(Context& context);

    virtual void waitIdle()                   = 0;
    virtual Queue& getQueue(Queue::Type type) = 0;

    Queue& getGraphicsQueue();
    Queue& getPresentQueue();

    virtual OwningPtr<Texture>
      createTexture(const Texture::ImageData& image, const Texture::SamplerProperties&) = 0;

    virtual OwningPtr<v2::RenderPass::Impl> createRenderPass(
      const v2::RenderPass::Properties& props
    ) = 0;

    virtual OwningPtr<Swapchain> createSwapchain(const Vec2<u32>& size) = 0;

    virtual OwningPtr<Semaphore> createSemaphore() = 0;

    virtual OwningPtr<Fence> createFence(
      Fence::State = Fence::State::notSignaled
    ) = 0;
};

}  // namespace sl

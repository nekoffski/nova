#pragma once

#include "starlight/core/Core.hh"
#include "starlight/core/Context.hh"
#include "starlight/core/memory/Memory.hh"

#include "fwd.hh"

#include "Queue.hh"
#include "Shader.hh"
#include "CommandBuffer.hh"
#include "Texture.hh"
#include "RenderPassBackend.hh"
#include "Buffer.hh"
#include "Pipeline.hh"

namespace sl {

class Device : public NonCopyable, public NonMovable {
public:
    virtual ~Device() = default;

    static OwningPtr<Device> create(Context& context);

    virtual void waitIdle()                   = 0;
    virtual Queue& getQueue(Queue::Type type) = 0;

    Queue& getGraphicsQueue();
    Queue& getPresentQueue();

    virtual OwningPtr<Buffer> createBuffer(const Buffer::Properties& props) = 0;

    virtual OwningPtr<Pipeline> createPipeline(
      Shader& shader, RenderPassBackend& renderPass
    ) = 0;

    virtual OwningPtr<Shader> createShader(const Shader::Properties& props) = 0;

    virtual OwningPtr<CommandBuffer> createCommandBuffer(
      CommandBuffer::Severity severity = CommandBuffer::Severity::primary
    ) = 0;

    virtual OwningPtr<Texture> createTexture(
      const Texture::ImageData& image,
      const Texture::SamplerProperties& = Texture::SamplerProperties::createDefault()
    ) = 0;

    virtual OwningPtr<RenderPassBackend> createRenderPassBackend(
      const RenderPassBackend::Properties& props, bool hasPreviousPass,
      bool hasNextPass
    ) = 0;

    virtual OwningPtr<Swapchain> createSwapchain(const Vec2<u32>& size) = 0;

    virtual OwningPtr<Semaphore> createSemaphore() = 0;

    virtual OwningPtr<Fence> createFence(
      Fence::State = Fence::State::notSignaled
    ) = 0;
};

}  // namespace sl

#pragma once

#include <vector>

#include "starlight/core/memory/Memory.hh"
#include "starlight/core/Core.hh"
#include "starlight/event/EventHandlerSentinel.hh"
#include "starlight/core/Concepts.hh"

#include "gpu/Device.hh"
#include "gpu/Swapchain.hh"
#include "gpu/Sync.hh"
#include "gpu/CommandBuffer.hh"
#include "gpu/Buffer.hh"

namespace sl {

class Renderer {
public:
    explicit Renderer();

    Swapchain& getSwapchain();
    Buffer& getVertexBuffer();
    Buffer& getIndexBuffer();

    template <typename Callback>
    requires Callable<Callback, void, CommandBuffer&, u8, u64>
    void renderFrame(Callback&& callback) {
        if (auto imageIndex = beginFrame(); imageIndex) [[likely]] {
            callback(*m_commandBuffers[*imageIndex], *imageIndex, ++m_frameNumber);
            endFrame(*imageIndex);
        }
    }

private:
    void createSyncPrimitives();
    void createBuffers();
    void initEventHandlers();

    void onWindowResize(const Vec2<u32>& size);

    Fence* getImageFence(u32 imageIndex);

    SingleCaller m_guard;

    std::optional<u8> beginFrame();
    void endFrame(u32 imageIndex);

    UniquePtr<Swapchain> m_swapchain;

    UniquePtr<Buffer> m_vertexBuffer;
    UniquePtr<Buffer> m_indexBuffer;

    u8 m_currentFrame;
    u8 m_maxFramesInFlight;
    u64 m_frameNumber;

    std::vector<UniquePtr<CommandBuffer>> m_commandBuffers;
    std::vector<UniquePtr<Semaphore>> m_imageAvailableSemaphores;
    std::vector<UniquePtr<Semaphore>> m_queueCompleteSemaphores;
    std::vector<UniquePtr<Fence>> m_frameFences;
    std::vector<Fence*> m_imageFences;

    EventHandlerSentinel m_eventSentinel;
    bool m_recreatingSwapchain;
    u64 m_framesSinceResize;
};

}  // namespace sl

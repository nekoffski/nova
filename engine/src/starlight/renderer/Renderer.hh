#pragma once

#include <vector>

#include "starlight/core/Core.hh"
#include "starlight/core/Context.hh"

#include "gpu/Device.hh"
#include "gpu/Swapchain.hh"
#include "gpu/Sync.hh"
#include "gpu/CommandBuffer.hh"

namespace sl {

class Renderer {
public:
    explicit Renderer(Context& context);

    Context& getContext();

    template <typename Callback>
    requires Callable<Callback, void, v2::CommandBuffer&, u8>
    void renderFrame(Callback&& callback) {
        if (auto imageIndex = beginFrame(); imageIndex) [[likely]] {
            callback();
            endFrame(*imageIndex);
        }
    }

private:
    void createSyncPrimitives();

    std::optional<u8> beginFrame();
    void endFrame(u8 imageIndex);

    Context& m_context;
    Window& m_window;

    OwningPtr<Device> m_device;
    OwningPtr<Swapchain> m_swapchain;

    u8 m_currentFrame;
    u8 m_maxFramesInFlight;

    std::vector<OwningPtr<v2::CommandBuffer>> m_commandBuffers;
    std::vector<OwningPtr<Semaphore>> m_imageAvailableSemaphores;
    std::vector<OwningPtr<Semaphore>> m_queueCompleteSemaphores;
    std::vector<OwningPtr<Fence>> m_frameFences;
};

}  // namespace sl

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

    template <typename Callback> void renderFrame(Callback&& callback) {
        if (auto imageIndex = beginFrame(); imageIndex) [[likely]] {
            callback();
            endFrame(*imageIndex);
        }
    }

private:
    std::optional<u8> beginFrame();
    void endFrame(u8 imageIndex);

    Context& m_context;
    Window& m_window;

    OwningPtr<Device> m_device;
    OwningPtr<Swapchain> m_swapchain;

    u8 m_currentFrame;
    u8 m_maxFramesInFlight;

    std::vector<LocalPtr<v2::CommandBuffer>> m_commandBuffers;
    std::vector<LocalPtr<Semaphore>> m_imageAvailableSemaphores;
    std::vector<LocalPtr<Semaphore>> m_queueCompleteSemaphores;
    std::vector<OwningPtr<Fence>> m_frameFences;
};

}  // namespace sl

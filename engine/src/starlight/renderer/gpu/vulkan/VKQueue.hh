#pragma once

#include "starlight/renderer/gpu/Queue.hh"

#include "Vulkan.hh"

namespace sl {

class VKQueue : public Queue {
public:
    explicit VKQueue(VkQueue handle);

    void submit(
      CommandBuffer& commandBuffer, Semaphore* waitSemaphore,
      Semaphore* signalSemaphore, Fence* fence
    ) override;

private:
    VkQueue m_handle;
};

}  // namespace sl
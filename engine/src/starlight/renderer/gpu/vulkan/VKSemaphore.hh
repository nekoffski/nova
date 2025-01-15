#pragma once

#include "Vulkan.hh"

#include "starlight/renderer/gpu/Sync.hh"

#include "fwd.hh"

namespace sl::vk {

class VKSemaphore : public Semaphore {
public:
    explicit VKSemaphore(VkDevice device, Allocator* allocator);
    ~VKSemaphore();

    VkSemaphore getHandle();
    VkSemaphore* getHandlePointer();

private:
    VkSemaphore m_handle;
    VkDevice m_device;
    Allocator* m_allocator;
};

VKSemaphore& toVk(Semaphore& semaphore);

}  // namespace sl::vk

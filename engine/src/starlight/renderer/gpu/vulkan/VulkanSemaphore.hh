#pragma once

#include "Vulkan.hh"

#include "starlight/renderer/gpu/Sync.hh"

#include "fwd.hh"

namespace sl::vk {

class VulkanSemaphore : public Semaphore {
public:
    explicit VulkanSemaphore(VulkanDevice& device);
    ~VulkanSemaphore();

    VkSemaphore getHandle();
    VkSemaphore* getHandlePointer();

private:
    VkSemaphore m_handle;
    VulkanDevice& m_device;
};

VulkanSemaphore& toVk(Semaphore& semaphore);

}  // namespace sl::vk

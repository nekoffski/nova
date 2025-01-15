#include "VulkanSemaphore.hh"

#include "VulkanDevice.hh"

namespace sl::vk {

VulkanSemaphore::VulkanSemaphore(VulkanDevice& device
) : m_handle(VK_NULL_HANDLE), m_device(device) {
    VkSemaphoreCreateInfo semaphoreCreateInfo;
    clearMemory(&semaphoreCreateInfo);
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VK_ASSERT(vkCreateSemaphore(
      m_device.logical.handle, &semaphoreCreateInfo, m_device.allocator, &m_handle
    ));
}

VulkanSemaphore::~VulkanSemaphore() {
    if (m_handle)
        vkDestroySemaphore(m_device.logical.handle, m_handle, m_device.allocator);
}

VkSemaphore VulkanSemaphore::getHandle() { return m_handle; }

VkSemaphore* VulkanSemaphore::getHandlePointer() { return &m_handle; }

VulkanSemaphore& toVk(Semaphore& semaphore) {
    return static_cast<VulkanSemaphore&>(semaphore);
}

}  // namespace sl::vk

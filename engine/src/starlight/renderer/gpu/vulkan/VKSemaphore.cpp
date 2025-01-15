#include "VKSemaphore.hh"

#include "VKContext.hh"

namespace sl::vk {

VKSemaphore::VKSemaphore(VkDevice device, Allocator* allocator) :
    m_handle(VK_NULL_HANDLE), m_device(device), m_allocator(allocator) {
    VkSemaphoreCreateInfo semaphoreCreateInfo;
    clearMemory(&semaphoreCreateInfo);
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VK_ASSERT(
      vkCreateSemaphore(m_device, &semaphoreCreateInfo, m_allocator, &m_handle)
    );
}

VKSemaphore::~VKSemaphore() {
    if (m_handle) vkDestroySemaphore(m_device, m_handle, m_allocator);
}

VkSemaphore VKSemaphore::getHandle() { return m_handle; }

VkSemaphore* VKSemaphore::getHandlePointer() { return &m_handle; }

VKSemaphore& toVk(Semaphore& semaphore) {
    return static_cast<VKSemaphore&>(semaphore);
}

}  // namespace sl::vk

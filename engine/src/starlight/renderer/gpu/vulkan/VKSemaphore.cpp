#include "VKSemaphore.hh"

#include "VKContext.hh"

namespace sl::vk {

namespace v2 {
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
}  // namespace v2

VKSemaphore::VKSemaphore(VKContext& context, VKLogicalDevice& device) :
    m_context(context), m_device(device), m_handle(VK_NULL_HANDLE) {
    VkSemaphoreCreateInfo semaphoreCreateInfo;
    clearMemory(&semaphoreCreateInfo);
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VK_ASSERT(vkCreateSemaphore(
      m_device.getHandle(), &semaphoreCreateInfo, m_context.getAllocator(), &m_handle
    ));
}

VKSemaphore::~VKSemaphore() {
    if (m_handle)
        vkDestroySemaphore(m_device.getHandle(), m_handle, m_context.getAllocator());
}

VkSemaphore VKSemaphore::getHandle() const { return m_handle; }

VkSemaphore* VKSemaphore::getHandlePointer() { return &m_handle; }

}  // namespace sl::vk

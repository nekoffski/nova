#include "VulkanFence.hh"

#include "VulkanDevice.hh"

#include "starlight/core/window/Window.hh"

namespace sl::vk {

VulkanFence::VulkanFence(VulkanDevice& device, State state) :
    m_handle(VK_NULL_HANDLE), m_device(device), m_state(state) {
    VkFenceCreateInfo fenceCreateInfo;
    clearMemory(&fenceCreateInfo);
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    if (state == VulkanFence::State::signaled)
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VK_ASSERT(vkCreateFence(
      m_device.logical.handle, &fenceCreateInfo, m_device.allocator, &m_handle
    ));
    LOG_TRACE("vkCreateFence: {}", static_cast<void*>(m_handle));
}

VulkanFence::~VulkanFence() {
    wait(u64Max);
    if (m_handle) {
        LOG_TRACE("vkDestroyFence: {}", static_cast<void*>(m_handle));
        vkDestroyFence(m_device.logical.handle, m_handle, m_device.allocator);
    }
}

VkFence VulkanFence::getHandle() { return m_handle; }

static void logError(VkResult result) {
    switch (result) {
        case VK_TIMEOUT:
            LOG_WARN("vk_fence_wait - Timed out");
            break;
        case VK_ERROR_DEVICE_LOST:
            LOG_ERROR("vk_fence_wait - VK_ERROR_DEVICE_LOST.");
            break;
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            LOG_ERROR("vk_fence_wait - VK_ERROR_OUT_OF_HOST_MEMORY.");
            break;
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            LOG_ERROR("vk_fence_wait - VK_ERROR_OUT_OF_DEVICE_MEMORY.");
            break;
        default:
            LOG_ERROR("vk_fence_wait - An unknown error has occurred.");
            break;
    }
}

bool VulkanFence::wait(Nanoseconds timeout) {
    if (m_state == State::signaled) return true;

    const auto result =
      vkWaitForFences(m_device.logical.handle, 1, &m_handle, true, timeout);

    if (result == VK_SUCCESS) {
        m_state = State::signaled;
        return true;
    }
    logError(result);
    return false;
}

void VulkanFence::reset() {
    if (m_state == State::signaled) {
        VK_ASSERT(vkResetFences(m_device.logical.handle, 1, &m_handle));
        m_state = State::notSignaled;
    }
}

VulkanFence& toVk(Fence& fence) { return static_cast<VulkanFence&>(fence); }

}  // namespace sl::vk

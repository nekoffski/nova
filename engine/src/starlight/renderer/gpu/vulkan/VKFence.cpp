#include "VKFence.hh"

#include "VKContext.hh"

#include "starlight/core/window/Window.hh"

namespace sl::vk {

VKFence::VKFence(VkDevice device, Allocator* allocator, State state) :
    Fence(state), m_handle(VK_NULL_HANDLE), m_device(device),
    m_allocator(allocator) {
    VkFenceCreateInfo fenceCreateInfo;
    clearMemory(&fenceCreateInfo);
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    if (state == VKFence::State::signaled)
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VK_ASSERT(vkCreateFence(m_device, &fenceCreateInfo, m_allocator, &m_handle));
}

VKFence::~VKFence() {
    wait(u64Max);
    if (m_handle) vkDestroyFence(m_device, m_handle, m_allocator);
}

VkFence VKFence::getHandle() { return m_handle; }

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

bool VKFence::wait(Nanoseconds timeout) {
    if (m_state == State::signaled) return true;

    const auto result = vkWaitForFences(m_device, 1, &m_handle, true, timeout);

    if (result == VK_SUCCESS) {
        m_state = State::signaled;
        return true;
    }
    logError(result);
    return false;
}

void VKFence::reset() {
    if (m_state == State::signaled) {
        VK_ASSERT(vkResetFences(m_device, 1, &m_handle));
        m_state = State::notSignaled;
    }
}

VKFence& toVk(Fence& fence) { return static_cast<VKFence&>(fence); }

}  // namespace sl::vk

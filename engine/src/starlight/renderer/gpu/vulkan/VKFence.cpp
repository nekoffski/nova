#include "VKFence.hh"

#include "VKContext.hh"

#include "starlight/core/window/Window.hh"

namespace sl::vk {

VkFenceCreateInfo createFenceCreateInfo(VKFence::State state) {
    VkFenceCreateInfo fenceCreateInfo;
    clearMemory(&fenceCreateInfo);
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    if (state == VKFence::State::signaled)
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    return fenceCreateInfo;
}

VKFence::VKFence(VKContext& context, VKLogicalDevice& device, State state) :
    m_context(context), m_device(device), m_state(state) {
    auto fenceCreateInfo = createFenceCreateInfo(state);

    VK_ASSERT(vkCreateFence(
      m_device.getHandle(), &fenceCreateInfo, m_context.getAllocator(), &m_handle
    ));
}

VKFence::~VKFence() {
    vkWaitForFences(m_device.getHandle(), 1, &m_handle, true, UINT64_MAX);
    if (m_handle)
        vkDestroyFence(m_device.getHandle(), m_handle, m_context.getAllocator());
}

VkFence VKFence::getHandle() { return m_handle; }

namespace {

void logError(VkResult result) {
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

}  // namespace

bool VKFence::wait(Nanoseconds timeout) {
    if (m_state == State::signaled) return true;

    const auto result =
      vkWaitForFences(m_device.getHandle(), 1, &m_handle, true, timeout);

    if (result == VK_SUCCESS) {
        m_state = State::signaled;
        return true;
    }

    logError(result);
    return false;
}

void VKFence::reset() {
    if (m_state == State::signaled) {
        VK_ASSERT(vkResetFences(m_device.getHandle(), 1, &m_handle));
        m_state = State::notSignaled;
    }
}

}  // namespace sl::vk

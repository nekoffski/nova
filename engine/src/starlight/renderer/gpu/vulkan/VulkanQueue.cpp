#include "VulkanQueue.hh"

#include "VulkanSwapchain.hh"
#include "VulkanSemaphore.hh"

namespace sl::vk {

VulkanQueue::VulkanQueue(VkQueue handle) : m_handle(handle) {}

bool VulkanQueue::submit(const SubmitInfo& submitInfo) { return true; }

void VulkanQueue::wait() { vkQueueWaitIdle(m_handle); }

bool VulkanQueue::present(const PresentInfo& presentInfo) {
    VkPresentInfoKHR vkPresentInfo;
    clearMemory(&vkPresentInfo);
    vkPresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    if (presentInfo.waitSemaphore != nullptr) {
        vkPresentInfo.waitSemaphoreCount = 1;
        vkPresentInfo.pWaitSemaphores =
          toVk(*presentInfo.waitSemaphore).getHandlePointer();
    }

    vkPresentInfo.swapchainCount = 1;
    vkPresentInfo.pSwapchains    = toVk(presentInfo.swapchain).getHandlePtr();
    vkPresentInfo.pImageIndices  = &presentInfo.imageIndex;
    vkPresentInfo.pResults       = 0;

    if (VkResult result = vkQueuePresentKHR(m_handle, &vkPresentInfo);
        result != VK_SUCCESS) {
        LOG_ERROR(
          "Failed to present swapchain image: {}", getResultString(result, true)
        );
        return false;
    }
    return true;
}

}  // namespace sl::vk

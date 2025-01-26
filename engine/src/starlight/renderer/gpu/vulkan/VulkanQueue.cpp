#include "VulkanQueue.hh"

#include "VulkanSwapchain.hh"
#include "VulkanSemaphore.hh"
#include "VulkanFence.hh"

namespace sl::vk {

VulkanQueue::VulkanQueue(VkQueue handle) : m_handle(handle) {}

bool VulkanQueue::submit(const SubmitInfo& submitInfo) {
    // TODO: this should be more configurable
    VkSubmitInfo vkSubmitInfo;
    clearMemory(&vkSubmitInfo);
    vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    vkSubmitInfo.commandBufferCount = 1;
    vkSubmitInfo.pCommandBuffers    = toVk(submitInfo.commandBuffer).getHandlePtr();

    if (submitInfo.signalSemaphore != nullptr) {
        vkSubmitInfo.signalSemaphoreCount = 1;
        vkSubmitInfo.pSignalSemaphores =
          toVk(*submitInfo.signalSemaphore).getHandlePointer();
    }

    if (submitInfo.waitSemaphore != nullptr) {
        vkSubmitInfo.waitSemaphoreCount = 1;
        vkSubmitInfo.pWaitSemaphores =
          toVk(*submitInfo.waitSemaphore).getHandlePointer();
    }

    vkSubmitInfo.pWaitDstStageMask = 0;

    VkPipelineStageFlags flags[1] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
    };
    vkSubmitInfo.pWaitDstStageMask = flags;

    const auto result = vkQueueSubmit(
      m_handle, 1, &vkSubmitInfo,
      submitInfo.fence != nullptr ? toVk(*submitInfo.fence).getHandle() : nullptr
    );

    if (result != VK_SUCCESS) {
        LOG_ERROR(
          "vkQueueSubmit failed with result: {}", getResultString(result, true)
        );
        return false;
    }
    return true;
}

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
        if (result != VK_ERROR_OUT_OF_DATE_KHR) {
            LOG_ERROR(
              "Failed to present swapchain image: {}", getResultString(result, true)
            );
        } else {
            LOG_INFO("Swapchain out of date, need to recreate");
        }
        return false;
    }
    return true;
}

VkQueue VulkanQueue::getHandle() { return m_handle; }

}  // namespace sl::vk

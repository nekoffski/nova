#include "VKCommandBuffer.hh"

#include "VKContext.hh"

namespace sl::vk {

VKCommandBuffer::VKCommandBuffer(
  VKLogicalDevice& device, VkCommandPool commandPool, Severity severity
) : m_device(device), m_commandPool(commandPool) {
    create(severity);
}

VKCommandBuffer::~VKCommandBuffer() { destroy(); }

VkCommandBufferAllocateInfo VKCommandBuffer::createAllocateInfo(Severity severity) {
    VkCommandBufferAllocateInfo allocateInfo;
    clearMemory(&allocateInfo);
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

    const auto level =
      severity == Severity::primary
        ? VK_COMMAND_BUFFER_LEVEL_PRIMARY
        : VK_COMMAND_BUFFER_LEVEL_SECONDARY;

    allocateInfo.commandPool        = m_commandPool;
    allocateInfo.level              = level;
    allocateInfo.commandBufferCount = 1;
    allocateInfo.pNext              = 0;

    return allocateInfo;
}

void VKCommandBuffer::create(Severity severity) {
    m_state = State::notAllocated;

    auto allocateInfo = createAllocateInfo(severity);
    VK_ASSERT(
      vkAllocateCommandBuffers(m_device.getHandle(), &allocateInfo, &m_handle)
    );

    m_state = State::ready;
}

void VKCommandBuffer::destroy() {
    LOG_TRACE("Destroying command buffer");

    if (m_handle) {
        vkFreeCommandBuffers(m_device.getHandle(), m_commandPool, 1, &m_handle);
        m_handle = VK_NULL_HANDLE;
    }
    m_state = State::notAllocated;
}

void VKCommandBuffer::begin(const BeginFlags& flags) {
    auto beginInfo = createCommandBufferBeginInfo(flags);
    VK_ASSERT(vkBeginCommandBuffer(m_handle, &beginInfo));
    m_state = State::recording;
}

void VKCommandBuffer::end() {
    VK_ASSERT(vkEndCommandBuffer(m_handle));
    m_state = State::recordingEnded;
}

void VKCommandBuffer::updateSubmitted() { m_state = State::submitted; }

void VKCommandBuffer::reset() { m_state = State::ready; }

// TODO: doesn't make sense, redesign to a static factor method
void VKCommandBuffer::createAndBeginSingleUse() {
    create(Severity::primary);

    begin(BeginFlags{
      .isSingleUse          = true,
      .isRenderpassContinue = false,
      .isSimultaneousUse    = false,
    });
}

VkSubmitInfo VKCommandBuffer::createSubmitQueueInfo() const {
    VkSubmitInfo submitInfo;
    submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext                = nullptr;
    submitInfo.commandBufferCount   = 1;
    submitInfo.pCommandBuffers      = &m_handle;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores    = nullptr;
    submitInfo.waitSemaphoreCount   = 0;
    submitInfo.pWaitSemaphores      = nullptr;
    submitInfo.pWaitDstStageMask    = 0;

    return submitInfo;
}

void VKCommandBuffer::setState(State state) { m_state = state; }

VkCommandBuffer VKCommandBuffer::getHandle() const { return m_handle; }

void VKCommandBuffer::endSingleUse(VkQueue queue) {
    // End the command buffer.
    end();

    // Submit the queue
    auto submitInfo = createSubmitQueueInfo();
    VK_ASSERT(vkQueueSubmit(queue, 1, &submitInfo, 0));

    // Wait for it to finish
    VK_ASSERT(vkQueueWaitIdle(queue));

    // Free the command buffer.
    destroy();
}

VkCommandBufferBeginInfo VKCommandBuffer::createCommandBufferBeginInfo(
  const BeginFlags& flags
) const {
    VkCommandBufferBeginInfo beginInfo;
    clearMemory(&beginInfo);
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (flags.isSingleUse)
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (flags.isRenderpassContinue)
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;

    if (flags.isSimultaneousUse)
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    return beginInfo;
}

}  // namespace sl::vk

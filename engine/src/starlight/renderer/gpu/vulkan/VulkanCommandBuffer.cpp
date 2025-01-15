#include "VulkanCommandBuffer.hh"

#include "VulkanDevice.hh"

namespace sl::vk {

VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice& device, Severity severity) :
    m_device(device) {
    VkCommandBufferAllocateInfo allocateInfo;
    clearMemory(&allocateInfo);
    allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

    const auto level =
      severity == Severity::primary
        ? VK_COMMAND_BUFFER_LEVEL_PRIMARY
        : VK_COMMAND_BUFFER_LEVEL_SECONDARY;

    allocateInfo.commandPool        = m_device.logical.graphicsCommandPool;
    allocateInfo.level              = level;
    allocateInfo.commandBufferCount = 1;

    VK_ASSERT(
      vkAllocateCommandBuffers(m_device.logical.handle, &allocateInfo, &m_handle)
    );
}

VulkanCommandBuffer::~VulkanCommandBuffer() {
    LOG_TRACE("Destroying command buffer");

    if (m_handle) {
        vkFreeCommandBuffers(
          m_device.logical.handle, m_device.logical.graphicsCommandPool, 1, &m_handle
        );
        m_handle = VK_NULL_HANDLE;
    }
}

static VkCommandBufferBeginInfo createCommandBufferBeginInfo(
  CommandBuffer::BeginFlags flags
) {
    VkCommandBufferBeginInfo beginInfo;
    clearMemory(&beginInfo);
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (isFlagEnabled(flags, CommandBuffer::BeginFlags::singleUse))
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (isFlagEnabled(flags, CommandBuffer::BeginFlags::isRenderpassContinue))
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;

    if (isFlagEnabled(flags, CommandBuffer::BeginFlags::simultaneousUse))
        beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    return beginInfo;
}

void VulkanCommandBuffer::begin(BeginFlags flags) {
    auto beginInfo = createCommandBufferBeginInfo(flags);
    VK_ASSERT(vkBeginCommandBuffer(m_handle, &beginInfo));
}

void VulkanCommandBuffer::end() { VK_ASSERT(vkEndCommandBuffer(m_handle)); }

VkCommandBuffer VulkanCommandBuffer::getHandle() const { return m_handle; }

void VulkanCommandBuffer::execute(const Command& command) {
    /*
        TODO: handle commands
    */
}

}  // namespace sl::vk

#include "VulkanCommandBuffer.hh"

#include "VulkanDevice.hh"
#include "VulkanBuffer.hh"

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
    LOG_TRACE("vkAllocateCommandBuffers: {}", static_cast<void*>(m_handle));
}

VulkanCommandBuffer::~VulkanCommandBuffer() {
    if (m_handle) {
        LOG_TRACE("vkFreeCommandBuffers: {}", static_cast<void*>(m_handle));
        vkFreeCommandBuffers(
          m_device.logical.handle, m_device.logical.graphicsCommandPool, 1, &m_handle
        );
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

VkCommandBuffer* VulkanCommandBuffer::getHandlePtr() { return &m_handle; }

VkCommandBuffer VulkanCommandBuffer::getHandle() { return m_handle; }

void VulkanCommandBuffer::execute(const Command& command) {
    auto visitor = Overload{
        [&](const BindVertexBufferCommand& cmd) {
            vkCmdBindVertexBuffers(
              m_handle, 0, 1,
              static_cast<VulkanBuffer&>(cmd.buffer).getHandlePointer(), &cmd.offset
            );
        },
        [&](const BindIndexBufferCommand& cmd) {
            vkCmdBindIndexBuffer(
              m_handle, static_cast<VulkanBuffer&>(cmd.buffer).getHandle(),
              cmd.offset, VK_INDEX_TYPE_UINT32
            );
        },
        [&](const DrawCommand& cmd) {
            vkCmdDraw(
              m_handle, cmd.vertexCount, cmd.instanceCount, cmd.firstVertex,
              cmd.firstInstance
            );
        },
        [&](const DrawIndexedCommand& cmd) {
            vkCmdDrawIndexed(
              m_handle, cmd.indexCount, cmd.instanceCount, cmd.firstIndex,
              cmd.vertexOffset, cmd.firstInstance
            );
        },
        [&](const SetViewportCommand& cmd) {
            VkViewport viewport;
            viewport.x        = static_cast<float>(cmd.offset.x);
            viewport.y        = static_cast<float>(cmd.size.h);
            viewport.width    = static_cast<float>(cmd.size.w);
            viewport.height   = -static_cast<float>(cmd.size.h);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            vkCmdSetViewport(m_handle, 0, 1, &viewport);
        },
        [&](const SetScissorsCommand& cmd) {
            VkRect2D scissor;
            scissor.offset.x      = cmd.offset.x;
            scissor.offset.y      = cmd.offset.y;
            scissor.extent.width  = cmd.size.w;
            scissor.extent.height = cmd.size.h;
            vkCmdSetScissor(m_handle, 0, 1, &scissor);
        }
    };

    std::visit(visitor, command);
}

VulkanCommandBuffer& toVk(CommandBuffer& buffer) {
    return static_cast<VulkanCommandBuffer&>(buffer);
}

}  // namespace sl::vk

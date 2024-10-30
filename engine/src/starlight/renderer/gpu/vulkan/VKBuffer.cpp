#include "VKBuffer.hh"

#include <kc/core/Log.h>

#include "VKContext.hh"
#include "VKCommandBuffer.hh"

namespace sl::vk {

VKBuffer::VKBuffer(
  VKContext& context, VKLogicalDevice& device, const VKBuffer::Properties& props
) :
    m_context(context), m_device(device), m_totalSize(props.size),
    m_usageFlags(props.usageFlags), m_memoryPropertyFlags(props.memoryPropertyFlags),
    m_useFreeList(props.useFreeList) {
    if (m_useFreeList) m_bufferFreeList.emplace(props.size);

    auto allocator = m_context.getAllocator();

    auto bufferCreateInfo = createBufferCreateInfo();

    VK_ASSERT(
      vkCreateBuffer(m_device.getHandle(), &bufferCreateInfo, allocator, &m_handle)
    );

    const auto memoryRequirements = getMemoryRequirements(m_handle);

    const auto memoryIndex = m_device.findMemoryIndex(
      memoryRequirements.memoryTypeBits, m_memoryPropertyFlags
    );

    ASSERT(memoryIndex.has_value(), "Could not find memory index for vulkan buffer");

    m_memoryIndex = *memoryIndex;

    const auto allocateInfo = createMemoryAllocateInfo(memoryRequirements);

    VK_ASSERT(
      vkAllocateMemory(m_device.getHandle(), &allocateInfo, allocator, &m_memory)
    );

    if (props.bindOnCreate) {
        static constexpr uint64_t zeroOffset = 0;
        bind(zeroOffset);
    }
}

VKBuffer::~VKBuffer() { destroy(); }

uint64_t VKBuffer::allocate(uint64_t size) {
    if (m_useFreeList) [[likely]] {
        const auto offset = m_bufferFreeList->allocateBlock(size);
        ASSERT(offset.has_value(), "Could not allocate buffer memory");
        LOG_TRACE(
          "Buffer_{} - Allocating {} bytes of memory, returned offset: {}", getId(),
          size, *offset
        );
        return *offset;
    }
    LOG_WARN(
      "Allocating from buffer that doesn't have free list, offset won't be valid"
    );
    return 0;
}

void VKBuffer::free(uint64_t size, uint64_t offset) {
    if (m_useFreeList) [[likely]] {
        m_bufferFreeList->freeBlock(size, offset);
        return;
    }
    LOG_WARN("Freeing memory on buffer that doesn't have free list, will skip");
}

VkMemoryAllocateInfo VKBuffer::createMemoryAllocateInfo(
  VkMemoryRequirements memoryRequirements
) const {
    VkMemoryAllocateInfo allocateInfo;
    allocateInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.pNext           = nullptr;
    allocateInfo.allocationSize  = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = (uint32_t)m_memoryIndex;

    return allocateInfo;
}

VkMemoryRequirements VKBuffer::getMemoryRequirements(VkBuffer buffer) const {
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(m_device.getHandle(), buffer, &memoryRequirements);

    return memoryRequirements;
}

VkBufferCreateInfo VKBuffer::createBufferCreateInfo() const {
    VkBufferCreateInfo bufferCreateInfo;
    bufferCreateInfo.flags       = 0;
    bufferCreateInfo.pNext       = nullptr;
    bufferCreateInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size        = m_totalSize;
    bufferCreateInfo.usage       = m_usageFlags;
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    return bufferCreateInfo;
}

void VKBuffer::destroy() {
    auto logicalDeviceHandle = m_device.getHandle();
    auto allocator           = m_context.getAllocator();

    if (m_memory) vkFreeMemory(logicalDeviceHandle, m_memory, allocator);
    if (m_handle) vkDestroyBuffer(logicalDeviceHandle, m_handle, allocator);
}

void VKBuffer::bind(uint64_t offset) {
    VK_ASSERT(vkBindBufferMemory(m_device.getHandle(), m_handle, m_memory, offset));
}

bool VKBuffer::resize(uint64_t size, VkQueue queue, VkCommandPool pool) {
    ASSERT(
      size > m_totalSize,
      "VKBuffer::resize(...) requires new size to be greater than the actual one"
    );

    if (m_useFreeList) m_bufferFreeList->resize(size);

    auto bufferCreateInfo = createBufferCreateInfo();

    auto logicalDeviceHandle = m_device.getHandle();
    auto allocator           = m_context.getAllocator();

    VkBuffer buffer;
    VK_ASSERT(
      vkCreateBuffer(logicalDeviceHandle, &bufferCreateInfo, allocator, &buffer)
    );

    auto memoryRequirements = getMemoryRequirements(buffer
    );  // TODO: pass VkBuffer here!

    auto allocateInfo = createMemoryAllocateInfo(memoryRequirements);

    // Allocate the memory.
    VkDeviceMemory memory;

    VK_ASSERT(
      vkAllocateMemory(logicalDeviceHandle, &allocateInfo, allocator, &memory)
    );
    VK_ASSERT(vkBindBufferMemory(logicalDeviceHandle, buffer, memory, 0));

    copyTo(
      pool, nullptr, queue, buffer,
      VkBufferCopy{ .srcOffset = 0, .dstOffset = 0, .size = m_totalSize }
    );

    m_device.waitIdle();

    destroy();
    m_totalSize = size;
    m_memory    = memory;
    m_handle    = buffer;

    return true;
}

void* VKBuffer::lockMemory(
  uint64_t offset, uint64_t size, VkMemoryPropertyFlags flags
) {
    void* data;
    VK_ASSERT(vkMapMemory(m_device.getHandle(), m_memory, offset, size, flags, &data)
    );

    return data;
}

void VKBuffer::unlockMemory() { vkUnmapMemory(m_device.getHandle(), m_memory); }

void VKBuffer::loadData(
  uint64_t offset, uint64_t size, VkMemoryPropertyFlags flags, const void* data
) {
    void* buffer;

    VK_ASSERT(
      vkMapMemory(m_device.getHandle(), m_memory, offset, size, flags, &buffer)
    );

    std::memcpy(buffer, data, size);

    unlockMemory();
}

void VKBuffer::copyTo(
  VkCommandPool pool, [[maybe_unused]] VkFence fence, VkQueue queue,
  VkBuffer destination, const VkBufferCopy& copyRegion
) {
    vkQueueWaitIdle(queue);

    VKCommandBuffer tempCommandBuffer{
        m_device, pool, VKCommandBuffer::Severity::primary
    };
    tempCommandBuffer.createAndBeginSingleUse();

    vkCmdCopyBuffer(
      tempCommandBuffer.getHandle(), m_handle, destination, 1, &copyRegion
    );

    tempCommandBuffer.endSingleUse(queue);
}

}  // namespace sl::vk

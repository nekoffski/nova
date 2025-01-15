#include "VulkanBuffer.hh"

#include <kc/core/Log.h>

#include "VulkanDevice.hh"

namespace sl::vk {

static VkBufferUsageFlagBits toVk(BufferUsage flags) {
    return static_cast<VkBufferUsageFlagBits>(flags);
}

static VkMemoryPropertyFlagBits toVk(MemoryProperty flags) {
    return static_cast<VkMemoryPropertyFlagBits>(flags);
}

static VkMemoryAllocateInfo createMemoryAllocateInfo(
  VkMemoryRequirements memoryRequirements, u32 memoryIndex
) {
    VkMemoryAllocateInfo allocateInfo;
    allocateInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.pNext           = nullptr;
    allocateInfo.allocationSize  = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = memoryIndex;

    return allocateInfo;
}

VulkanBuffer::VulkanBuffer(VulkanDevice& device, const Properties& props) :
    m_device(device), m_props(props), m_freeList(props.size) {
    auto bufferCreateInfo = createBufferCreateInfo();

    VK_ASSERT(vkCreateBuffer(
      m_device.logical.handle, &bufferCreateInfo, m_device.allocator, &m_handle
    ));

    const auto memoryRequirements = getMemoryRequirements();

    const auto memoryIndex = m_device.findMemoryIndex(
      memoryRequirements.memoryTypeBits, toVk(m_props.memoryProperty)
    );
    ASSERT(memoryIndex.has_value(), "Could not find memory index for vulkan buffer");
    m_memoryIndex = *memoryIndex;

    const auto allocateInfo =
      createMemoryAllocateInfo(memoryRequirements, *memoryIndex);

    VK_ASSERT(vkAllocateMemory(
      m_device.logical.handle, &allocateInfo, m_device.allocator, &m_memory
    ));

    if (props.bindOnCreate) bind();
}

VulkanBuffer::~VulkanBuffer() {
    auto device    = m_device.logical.handle;
    auto allocator = m_device.allocator;

    if (m_memory) vkFreeMemory(device, m_memory, allocator);
    if (m_handle) vkDestroyBuffer(device, m_handle, allocator);
}

std::optional<u64> VulkanBuffer::allocate(u64 size) {
    return m_freeList.allocateBlock(size);
}

void VulkanBuffer::free(u64 size, u64 offset) { m_freeList.freeBlock(size, offset); }

VkMemoryRequirements VulkanBuffer::getMemoryRequirements() const {
    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(
      m_device.logical.handle, m_handle, &memoryRequirements
    );
    return memoryRequirements;
}

VkBufferCreateInfo VulkanBuffer::createBufferCreateInfo() const {
    VkBufferCreateInfo bufferCreateInfo;
    bufferCreateInfo.flags       = 0;
    bufferCreateInfo.pNext       = nullptr;
    bufferCreateInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferCreateInfo.size        = m_props.size;
    bufferCreateInfo.usage       = toVk(m_props.usage);
    bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    return bufferCreateInfo;
}

void VulkanBuffer::bind(u64 offset) {
    VK_ASSERT(vkBindBufferMemory(m_device.logical.handle, m_handle, m_memory, offset)
    );
}

void* VulkanBuffer::lockMemory(u64 offset, u64 size, MemoryProperty flags) {
    void* data;
    VK_ASSERT(vkMapMemory(
      m_device.logical.handle, m_memory, offset, size, toVk(flags), &data
    ));
    return data;
}

void VulkanBuffer::unlockMemory() {
    vkUnmapMemory(m_device.logical.handle, m_memory);
}

void VulkanBuffer::loadData(
  u64 offset, u64 size, MemoryProperty flags, const void* data
) {
    std::memcpy(lockMemory(offset, size, flags), data, size);
    unlockMemory();
}

}  // namespace sl::vk

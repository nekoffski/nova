#include "VulkanBuffer.hh"

#include "VulkanDevice.hh"
#include "VulkanCommandBuffer.hh"

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

    log::expect(vkCreateBuffer(
      m_device.logical.handle, &bufferCreateInfo, m_device.allocator, &m_handle
    ));
    log::trace("vkCreateBuffer: {}", static_cast<void*>(m_handle));

    const auto memoryRequirements = getMemoryRequirements();

    const auto memoryIndex = m_device.findMemoryIndex(
      memoryRequirements.memoryTypeBits, toVk(m_props.memoryProperty)
    );
    log::expect(
      memoryIndex.has_value(), "Could not find memory index for vulkan buffer"
    );
    m_memoryIndex = *memoryIndex;

    const auto allocateInfo =
      createMemoryAllocateInfo(memoryRequirements, *memoryIndex);

    log::expect(vkAllocateMemory(
      m_device.logical.handle, &allocateInfo, m_device.allocator, &m_memory
    ));
    log::trace("vkAllocateMemory: {}", static_cast<void*>(m_memory));

    if (props.bindOnCreate) bind();
}

VulkanBuffer::~VulkanBuffer() {
    auto device    = m_device.logical.handle;
    auto allocator = m_device.allocator;

    m_device.waitIdle();

    if (m_memory) {
        log::trace("vkFreeMemory: {}", static_cast<void*>(m_memory));
        vkFreeMemory(device, m_memory, allocator);
    }
    if (m_handle) {
        log::trace("vkDestroyBuffer: {}", static_cast<void*>(m_handle));
        vkDestroyBuffer(device, m_handle, allocator);
    }
}

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
    log::expect(
      vkBindBufferMemory(m_device.logical.handle, m_handle, m_memory, offset)
    );
}

void* VulkanBuffer::lockMemory(const Range& range) {
    void* data;
    log::expect(vkMapMemory(
      m_device.logical.handle, m_memory, range.offset,
      range.size == max<u64>() ? VK_WHOLE_SIZE : range.size, 0, &data
    ));
    return data;
}

void VulkanBuffer::unlockMemory() {
    vkUnmapMemory(m_device.logical.handle, m_memory);
}

std::optional<Range> VulkanBuffer::allocate(u64 size, const void* data) {
    auto offset = m_freeList.allocateBlock(size);

    if (not offset) {
        log::warn("Could not allocate {}b, not space left", size);
        return {};
    }
    Range range{ .offset = *offset, .size = size };

    if (data != nullptr) {
        const auto isDeviceLocal = isFlagEnabled(
          m_props.memoryProperty, MemoryProperty::MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        if (isDeviceLocal) {
            VulkanBuffer stagingBuffer{
                m_device, Buffer::Properties::staging(range.size)
            };
            stagingBuffer.copy(Range{ .offset = 0u, .size = range.size }, data);

            VkBufferCopy copyRegion{
                .srcOffset = 0, .dstOffset = range.offset, .size = size
            };
            ImmediateCommandBuffer commandBuffer{
                m_device.createCommandBuffer(), m_device.getGraphicsQueue()
            };
            vkCmdCopyBuffer(
              toVk(commandBuffer).getHandle(), stagingBuffer.getHandle(), m_handle,
              1, &copyRegion
            );
        } else {
            copy(range, data);
        }
    }

    return range;
}

void VulkanBuffer::free(const Range& range) {
    m_freeList.freeBlock(range.size, range.offset);
}

void VulkanBuffer::copy(const Range& range, const void* data) {
    auto destMemory = lockMemory(range);
    std::memcpy(destMemory, data, range.size);
    unlockMemory();
}

}  // namespace sl::vk

#pragma once

#include "starlight/core/memory/Memory.hh"
#include "starlight/core/utils/FreeList.hh"

#include "starlight/renderer/gpu/Buffer.hh"

#include "VulkanCommandBuffer.hh"
#include "Vulkan.hh"
#include "fwd.hh"

namespace sl::vk {

class VulkanBuffer : Buffer {
public:
    explicit VulkanBuffer(VulkanDevice& device, const Properties& props);
    ~VulkanBuffer();

    VkBuffer getHandle() const { return m_handle; }
    VkBuffer* getHandlePointer() { return &m_handle; }

    void bind(u64 offset = 0) override;

    void* lockMemory(u64 offset, u64 size, MemoryProperty memoryProperty) override;
    void unlockMemory() override;

    std::optional<u64> allocate(u64 size) override;
    void free(u64 size, u64 offset) override;

    void loadData(
      u64 offset, u64 size, MemoryProperty memoryProperty, const void* data
    ) override;

private:
    VkBufferCreateInfo createBufferCreateInfo() const;
    VkMemoryRequirements getMemoryRequirements() const;

    VulkanDevice& m_device;
    Properties m_props;
    FreeList m_freeList;

    VkBuffer m_handle;
    VkDeviceMemory m_memory;
    bool m_isLocked;

    i32 m_memoryIndex;
};

}  // namespace sl::vk

#pragma once

#include "starlight/core/memory/Memory.hh"
#include "starlight/core/containers/FreeList.hh"

#include "starlight/renderer/gpu/Buffer.hh"

#include "VulkanCommandBuffer.hh"
#include "Vulkan.hh"
#include "fwd.hh"

namespace sl::vk {

class VulkanBuffer : public Buffer {
public:
    explicit VulkanBuffer(VulkanDevice& device, const Properties& props);
    ~VulkanBuffer();

    VkBuffer getHandle() const { return m_handle; }
    VkBuffer* getHandlePointer() { return &m_handle; }

    void bind(u64 offset = 0) override;

    void* lockMemory(const Range& range = Range{ 0u, max<u64>() }) override;
    void unlockMemory() override;

    std::optional<Range> allocate(u64 size, const void* data = nullptr) override;
    void free(const Range& range) override;

    void copy(const Range& range, const void* data) override;

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

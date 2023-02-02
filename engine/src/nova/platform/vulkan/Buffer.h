#pragma once

#include <cstdint>

#include "Vulkan.h"
#include "fwd.h"

namespace nova::platform::vulkan {

class Buffer {
   public:
    struct Properties {
        uint64_t size;
        VkMemoryPropertyFlags memoryPropertyFlags;
        VkBufferUsageFlags usageFlags;

        bool bindOnCreate;
    };

    explicit Buffer(const Context* context, const Device* device, const Properties& props);
    ~Buffer();

    VkBuffer getHandle() const { return m_handle; }

    VkBuffer* getHandlePointer() { return &m_handle; }

    void destroy();

    void bind(uint64_t offset);

    bool resize(uint64_t size, VkQueue queue, VkCommandPool pool);

    void* lockMemory(uint64_t offset, uint64_t size, VkMemoryPropertyFlags flags);

    void unlockMemory();

    void loadData(uint64_t offset, uint64_t size, VkMemoryPropertyFlags flags, const void* data);

    void copyTo(
        VkCommandPool pool, VkFence fence, VkQueue queue, VkBuffer destination,
        const VkBufferCopy& copyRegion
    );

   private:
    VkBufferCreateInfo createBufferCreateInfo() const;

    VkMemoryRequirements getMemoryRequirements(VkBuffer buffer) const;

    VkMemoryAllocateInfo createMemoryAllocateInfo(VkMemoryRequirements memoryRequirements) const;

    const Context* m_context;
    const Device* m_device;

    uint64_t m_totalSize;

    VkBuffer m_handle;
    VkBufferUsageFlags m_usageFlags;

    bool m_isLocked;

    VkDeviceMemory m_memory;

    int32_t m_memoryIndex;
    uint32_t m_memoryPropertyFlags;
};

}  // namespace nova::platform::vulkan
#pragma once

#include <vector>

#include "starlight/core/memory/Memory.hh"

#include "VKCommandBuffer.hh"
#include "VKLogicalDevice.hh"

#include "starlight/renderer/gpu/CommandBufferPool.hh"

namespace sl::vk {

class VKCommandBufferPool : public CommandBufferPool {
public:
    explicit VKCommandBufferPool(
      [[maybe_unused]] VKContext& context, VKLogicalDevice& device, u64 size
    ) : m_device(device), m_frameImageIndex(0u), m_size(size) {
        create();
    }

    void recreate() {
        m_commandBuffers.clear();
        create();
    }

    void setFrameImageIndex(u32 frameImageIndex) {
        m_frameImageIndex = frameImageIndex;
    }

    VKCommandBuffer* getFrameCommandBuffer() override {
        return m_commandBuffers[m_frameImageIndex].get();
    }

private:
    void create() {
        m_commandBuffers.reserve(m_size);

        for (u64 i = 0; i < m_size; ++i) {
            m_commandBuffers.emplace_back(
              m_device, m_device.getGraphicsCommandPool()
            );
        }
    }

    VKLogicalDevice& m_device;

    u32 m_frameImageIndex;
    u64 m_size;

    std::vector<LocalPtr<VKCommandBuffer>> m_commandBuffers;
};

}  // namespace sl::vk
#pragma once

#include "Vulkan.hh"

#include "starlight/renderer/gpu/CommandBuffer.hh"
#include "fwd.hh"

namespace sl::vk {

class VulkanCommandBuffer : public CommandBuffer {
public:
    explicit VulkanCommandBuffer(
      VulkanDevice& device, Severity severity = Severity::primary
    );

    ~VulkanCommandBuffer();

    void begin(BeginFlags args) override;
    void end() override;
    void execute(const Command& command) override;

    VkCommandBuffer getHandle() const;

private:
    VulkanDevice& m_device;
    VkCommandBuffer m_handle;
};
}  // namespace sl::vk

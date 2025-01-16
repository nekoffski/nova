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

    VkCommandBuffer* getHandlePtr();
    VkCommandBuffer getHandle();

private:
    VulkanDevice& m_device;
    VkCommandBuffer m_handle;
};

VulkanCommandBuffer& toVk(CommandBuffer&);

}  // namespace sl::vk

#pragma once

#include <span>
#include <vector>

#include "starlight/core/Core.hh"

#include "Vulkan.hh"
#include "fwd.hh"

#include "starlight/renderer/gpu/Pipeline.hh"
#include "starlight/renderer/gpu/fwd.hh"

namespace sl::vk {

class VulkanPipeline : public Pipeline {
public:
    explicit VulkanPipeline(
      VulkanDevice& device, VulkanShader& shader,
      VulkanRenderPassBackend& renderPass, const Properties& props
    );

    ~VulkanPipeline() override;

    void bind(CommandBuffer& commandBuffer) override;

    VkPipelineLayout getLayout() const;

private:
    VulkanDevice& m_device;
    VkPipelineLayout m_layout;
    VkPipeline m_handle;
};

}  // namespace sl::vk

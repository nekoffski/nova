#pragma once

#include <span>
#include <vector>

#include "starlight/core/Log.hh"
#include "starlight/core/Core.hh"

#include "Vulkan.hh"

#include "VulkanCommandBuffer.hh"
#include "VulkanRenderPassBackend.hh"

#include "starlight/core/math/Vertex.hh"

#include "starlight/renderer/gpu/Pipeline.hh"
#include "starlight/renderer/gpu/fwd.hh"

namespace sl::vk {

// TODO: REFACTOR

class VulkanPipeline : public Pipeline {
public:
    struct Properties {
        uint32_t stride;
        std::vector<VkVertexInputAttributeDescription> vertexAttributes;
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
        std::vector<VkPipelineShaderStageCreateInfo> stages;
        std::vector<Range> pushConstantRanges;
        VkViewport viewport;
        VkRect2D scissor;
        VkPolygonMode polygonMode;
        bool depthTestEnabled;
        CullMode cullMode;
    };

    explicit VulkanPipeline(
      VkDevice device, Allocator* allocator, Shader& shader,
      VulkanRenderPassBackend& renderPass
    );

    ~VulkanPipeline() {
        LOG_TRACE("Vulkan pipeline destroyed");
        vkDeviceWaitIdle(m_device);
        vkDestroyPipeline(m_device, m_handle, m_allocator);
        vkDestroyPipelineLayout(m_device, m_layout, m_allocator);
    }

    void bind(CommandBuffer& commandBuffer) override {
        vkCmdBindPipeline(
          static_cast<VulkanCommandBuffer&>(commandBuffer).getHandle(),
          VK_PIPELINE_BIND_POINT_GRAPHICS, m_handle
        );
    }

    VkPipelineLayout getLayout() const { return m_layout; }

private:
    VkDevice m_device;
    Allocator* m_allocator;

    VkPipelineLayout m_layout = VK_NULL_HANDLE;
    VkPipeline m_handle       = VK_NULL_HANDLE;
};

}  // namespace sl::vk

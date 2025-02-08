#pragma once

#include "Vulkan.hh"

#include "starlight/renderer/gpu/ShaderDataBinder.hh"
#include "fwd.hh"

#include "VulkanBuffer.hh"

namespace sl::vk {

class VulkanShaderDataBinder : public ShaderDataBinder {
    // TODO: this should be defined in one place
    static constexpr u32 maxFramesInFlight = 3u;

public:
    explicit VulkanShaderDataBinder(VulkanDevice& device, VulkanShader& shader);

    u32 acquireLocalDescriptor() override;
    void releaseLocalDescriptor(u32 id) override;

private:
    void bindGlobalDescriptor() override;
    void bindLocalDescriptor(u32 id) override;

    void updateGlobalDescriptor(
      CommandBuffer& commandBuffer, u32 imageIndex, Pipeline& pipeline
    ) override;
    void updateLocalDescriptor(
      CommandBuffer& commandBuffer, u32 imageIndex, Pipeline& pipeline
    ) override;

    void setLocalSampler(const Shader::Uniform& uniform, const Texture* value)
      override;

    void setGlobalSampler(const Shader::Uniform& uniform, const Texture* value)
      override;

    void setLocalUniform(const Shader::Uniform& uniform, const void* value) override;

    void setGlobalUniform(const Shader::Uniform& uniform, const void* value)
      override;

    void setPushConstant(
      const Shader::Uniform& uniform, const void* value,
      CommandBuffer& commandBuffer, Pipeline& pipeline
    ) override;

    void createDescriptorPool();
    void createUniformBuffer();

    VulkanDevice& m_device;
    VulkanShader& m_shader;

    VkDescriptorPool m_descriptorPool;

    u64 m_globalUboStride;
    u64 m_localUboStride;
    u64 m_globalUboOffset;

    LocalPointer<VulkanBuffer> m_uniformBuffer;
    void* m_uniformBufferView;

    std::vector<VkDescriptorSet> m_globalDescriptorSets;
};

}  // namespace sl::vk

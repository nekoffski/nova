#pragma once

#include "Vulkan.hh"

#include "starlight/renderer/gpu/ShaderDataBinder.hh"
#include "fwd.hh"

#include "VulkanBuffer.hh"

namespace sl::vk {

class VulkanShaderDataBinder : public ShaderDataBinder {
    static constexpr u8 maxFramesInFlight       = 3u;
    static constexpr u32 maxLocalDescriptorSets = 1024u;
    // TODO: this should be defined in one place

    struct LocalDescriptorSet {
        explicit LocalDescriptorSet(u32 id, u32 textureCount);

        u32 id;
        u64 offset;
        std::array<VkDescriptorSet, maxFramesInFlight> descriptorSets;
        std::vector<const VulkanTexture*> textures;
    };

    using LocalDescriptorSets =
      std::array<LocalPointer<LocalDescriptorSet>, maxLocalDescriptorSets>;

public:
    explicit VulkanShaderDataBinder(VulkanDevice& device, VulkanShader& shader);
    ~VulkanShaderDataBinder() override;

    u32 acquireLocalDescriptorSet() override;
    void releaseLocalDescriptorSet(u32 id) override;

private:
    void updateGlobalDescriptorSet(
      CommandBuffer& commandBuffer, u32 imageIndex, Pipeline& pipeline
    ) override;
    void updateLocalDescriptorSet(
      CommandBuffer& commandBuffer, u32 id, u32 imageIndex, Pipeline& pipeline
    ) override;

    void setLocalSampler(
      const Shader::Uniform& uniform, u32 id, const Texture* value
    ) override;

    void setGlobalSampler(const Shader::Uniform& uniform, const Texture* value)
      override;

    void setLocalUniform(const Shader::Uniform& uniform, u32 id, const void* value)
      override;

    void setGlobalUniform(const Shader::Uniform& uniform, const void* value)
      override;

    void setUniform(const Range& range, const void* value);

    void setPushConstant(
      const Shader::Uniform& uniform, const void* value,
      CommandBuffer& commandBuffer, Pipeline& pipeline
    ) override;

    void createDescriptorPool();
    void createUniformBuffer();

    LocalDescriptorSet* findFreeLocalDescriptorSet();

    VulkanDevice& m_device;
    VulkanShader& m_shader;

    VkDescriptorPool m_descriptorPool;

    u64 m_globalUboStride;
    u64 m_localUboStride;
    u64 m_globalUboOffset;

    LocalPointer<VulkanBuffer> m_uniformBuffer;
    void* m_uniformBufferView;

    std::vector<VkDescriptorSet> m_globalDescriptorSets;
    std::vector<const VulkanTexture*> m_globalTextures;
    LocalDescriptorSets m_localDescriptorSets;
};

}  // namespace sl::vk

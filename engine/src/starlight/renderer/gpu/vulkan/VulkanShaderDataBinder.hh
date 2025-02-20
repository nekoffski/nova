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
        u64 lastUpdateFrame;
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
    void bindGlobalDescriptorSet(
      CommandBuffer& commandBuffer, u64 frameNumber, u32 imageIndex,
      Pipeline& pipeline, bool update
    ) override;

    void bindLocalDescriptorSet(
      CommandBuffer& commandBuffer, u64 frameNumber, u32 id, u32 imageIndex,
      Pipeline& pipeline, bool update
    ) override;

    void bindDescriptorSet(
      CommandBuffer& commandBuffer, Pipeline& pipeline,
      VkDescriptorSet& descriptorSet, u64 uniformBufferOffset, u64 stride,
      std::span<const VulkanTexture*> textures, u64 nonSamplerCount,
      u64 descriptorIndex, u8& counter
    );

    bool setLocalSampler(
      const Shader::Uniform& uniform, u32 id, const Texture* value
    ) override;

    bool setGlobalSampler(const Shader::Uniform& uniform, const Texture* value)
      override;

    bool setLocalUniform(const Shader::Uniform& uniform, u32 id, const void* value)
      override;

    bool setGlobalUniform(const Shader::Uniform& uniform, const void* value)
      override;

    bool setUniform(const Range& range, const void* value);

    void setPushConstant(
      const Shader::Uniform& uniform, const void* value,
      CommandBuffer& commandBuffer, Pipeline& pipeline
    ) override;

    void createDescriptorPool();
    void createUniformBuffer();

    LocalDescriptorSet* findFreeLocalDescriptorSet();

    VulkanDevice& m_device;
    VulkanShader& m_shader;
    const Shader::DataLayout& m_dataLayout;

    VkDescriptorPool m_descriptorPool;

    u8 m_localDescriptorDirtyFrames;
    u8 m_globalDescriptorDirtyFrames;

    u64 m_globalUboStride;
    u64 m_localUboStride;
    u64 m_globalUboOffset;

    LocalPointer<VulkanBuffer> m_uniformBuffer;
    void* m_uniformBufferView;

    std::vector<VkDescriptorSet> m_globalDescriptorSets;
    std::vector<const VulkanTexture*> m_globalTextures;
    u64 m_globalLastUpdateFrame;
    LocalDescriptorSets m_localDescriptorSets;
};

}  // namespace sl::vk

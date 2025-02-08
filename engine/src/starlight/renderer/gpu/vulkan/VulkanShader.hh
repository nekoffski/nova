#pragma once

#include <string>
#include <optional>

#include "starlight/renderer/gpu/Shader.hh"

#include "Vulkan.hh"
#include "VulkanDevice.hh"

namespace sl::vk {

class VulkanShader : public Shader {
    struct Bindings {
        u8 count = 0u;
        std::optional<u8> ubo;
        std::optional<u8> sampler;
    };

public:
    using PipelineStageInfos = std::vector<VkPipelineShaderStageCreateInfo>;
    using InputAttributesDescriptions =
      std::vector<VkVertexInputAttributeDescription>;

    explicit VulkanShader(
      VulkanDevice& device, const Shader::Properties& properties
    );

    ~VulkanShader() override;

    const PipelineStageInfos& getPipelineStageInfos() const;
    const InputAttributesDescriptions& getInputAttributesDescriptions() const;
    const std::vector<VkDescriptorSetLayout>& getDescriptorSetLayouts() const;
    const std::vector<VkShaderModule> getModules() const;

    const Bindings& getDescriptorSetBindings(Uniform::Scope scope) const;

private:
    void prepareAttributeDescriptions();
    void createDescriptorSetLayouts();
    void createDescriptorSetLayout(Uniform::Scope scope);

    void processStages();
    void processStage(const Shader::Stage& stage);

    VulkanDevice& m_device;

    std::array<Bindings, descriptorSetCount> m_descriptorSetsBindings;

    std::vector<VkShaderModule> m_modules;
    PipelineStageInfos m_pipelineStageInfos;
    InputAttributesDescriptions m_attributeDescriptions;
    std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
};

}  // namespace sl::vk

// class VulkanShaderStage {
// public:
//     struct Properties {
//         const std::string& source;
//         Shader::Stage::Type type;
//     };

//     explicit VulkanShaderStage(VulkanDevice& device, const Properties&
//     properties);

//     VkPipelineShaderStageCreateInfo getStageCreateInfo() const {
//         return m_stageCreateInfo;
//     }

//     ~VulkanShaderStage();

// private:
//     VulkanDevice& m_device;

//     VkShaderModuleCreateInfo m_moduleCreateInfo;
//     VkPipelineShaderStageCreateInfo m_stageCreateInfo;
//     VkShaderModule m_handle;
// };

// class VulkanShader final : public Shader {
//     static constexpr u32 maxBindings = 2;

//     struct DescriptorSetConfig {
//         DescriptorSetConfig() : bindingCount(0), samplerBindingIndex(0) {
//             for (auto& binding : bindings) clearMemory(&binding);
//         }

//         u8 bindingCount;
//         Id8 samplerBindingIndex;
//         std::array<VkDescriptorSetLayoutBinding, maxBindings> bindings;
//     };

//     struct DescriptorState {
//         // 1 per frame
//         std::array<Id8, 3> generations;
//         std::array<Id32, 3> ids;
//     };

//     struct DescriptorSetState {
//         std::array<VkDescriptorSet, 3> descriptorSets;
//         std::vector<DescriptorState> descriptorStates;
//     };

//     struct InstanceState {
//         Id32 id;
//         Id64 offset;
//         DescriptorSetState descriptorSetState;
//         std::vector<const Texture*> instanceTextures;
//     };

// public:
//     explicit VulkanShader(VulkanDevice& device, const Shader::Properties& props);
//     ~VulkanShader() override;

//     // clang-format off
//     void use(CommandBuffer&) override;
//     u32 acquireInstanceResources(
//         const std::vector<Texture*>& textures
//     ) override;
//     void releaseInstanceResources(u32 instanceId) override;
//     // clang-format on

//     void bindPipeline(Pipeline& pipeline) override;

//     VulkanPipeline::Properties getPipelineProperties();

// private:
//     void setSampler(const std::string& uniform, const Texture* value) override;
//     void setUniform(
//       const std::string& name, const void* value, CommandBuffer& commandBuffer
//     ) override;

//     void bindInstance(u32 instanceId) override;

//     void bindGlobals() override;
//     void applyGlobals(CommandBuffer& commandBuffer, u32 imageIndex) override;
//     void applyInstance(CommandBuffer& commandBuffer, u32 imageIndex) override;

//     void createModules(std::span<const Stage> stages);
//     void processAttributes();
//     void createDescriptorPool();
//     void createDescriptorSetLayouts();
//     void createUniformBuffer();

//     void addAttributes(std::span<const Shader::Attribute> attributes);
//     void addUniforms(
//       std::span<const Shader::Uniform::Properties> uniforms, Texture*
//       defaultTexture
//     );
//     void addUniform(
//       const std::string& name, u32 size, Uniform::Type type, Scope scope,
//       u32 setLocation, bool isSampler
//     );
//     void addSampler(
//       const Shader::Uniform::Properties& props, Texture* defaultTexture
//     );

//     VulkanDevice& m_device;

//     void* m_mappedUniformBufferBlock;
//     VkDescriptorPool m_descriptorPool;
//     std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
//     std::array<VkDescriptorSet, 3> m_globalDescriptorSets;

//     VulkanPipeline* m_pipeline;
//     LocalPointer<VulkanBuffer> m_uniformBuffer;

//     std::vector<InstanceState> m_instanceStates;
//     std::vector<VulkanShaderStage> m_stages;

//     u64 m_requiredUboAlignment;

//     u64 m_globalUboSize;
//     u64 m_globalUboStride;
//     u64 m_globalUboOffset;

//     u64 m_uboSize;
//     u64 m_uboStride;

//     u64 m_pushConstantSize;
//     u64 m_pushConstantStride;

//     std::vector<const Texture*> m_globalTextures;
//     u8 m_instanceTextureCount;

//     u32 m_boundInstanceId;
//     u32 m_boundUboOffset;

//     std::unordered_map<std::string, Shader::Uniform> m_uniforms;
//     std::vector<Shader::Attribute> m_attributes;

//     u8 m_pushConstantRangeCount;
//     std::array<Range, 32> m_pushConstantRanges;

//     u16 m_attributeStride;

//     std::array<VkDescriptorPoolSize, 2> m_poolSizes;
//     u16 m_maxDescriptorSetCount;
//     u8 m_descriptorSetCount;
//     std::vector<DescriptorSetConfig> m_descriptorSets;
//     std::vector<VkVertexInputAttributeDescription> m_attributeDescriptions;

//     u8 m_globalUniformCount;
//     u8 m_globalUniformSamplerCount;
//     u8 m_instanceUniformCount;
//     u8 m_instanceUniformSamplerCount;
//     u8 m_localUniformCount;
// };

// }  // namespace sl::vk
#pragma once

#include <string>
#include <optional>
#include <vector>

#include "starlight/renderer/gpu/Shader.hh"

#include "Vulkan.hh"
#include "VulkanDevice.hh"
#include "VulkanTexture.hh"

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
      VulkanDevice& device, const Shader::Properties& properties, OptStr name
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

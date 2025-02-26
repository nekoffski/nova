#include "VulkanShader.hh"

namespace sl::vk {

static VkFormat toVk(Shader::DataType type) {
    switch (type) {
        case Shader::DataType::f32:
            return VK_FORMAT_R32_SFLOAT;
        case Shader::DataType::vec2:
            return VK_FORMAT_R32G32_SFLOAT;
        case Shader::DataType::vec3:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case Shader::DataType::vec4:
            return VK_FORMAT_R32G32B32A32_SFLOAT;
        case Shader::DataType::i8:
            return VK_FORMAT_R8_SINT;
        case Shader::DataType::u8:
            return VK_FORMAT_R8_UINT;
        case Shader::DataType::i16:
            return VK_FORMAT_R16_SINT;
        case Shader::DataType::u16:
            return VK_FORMAT_R16_UINT;
        case Shader::DataType::i32:
            return VK_FORMAT_R32_SINT;
        case Shader::DataType::u32:
            return VK_FORMAT_R32_UINT;
        default:
            break;
    }
    log::panic("Invalid data type: {}", fmt::underlying(type));
}

static VkShaderStageFlagBits toVk(Shader::Stage::Type type) {
    switch (type) {
        case Shader::Stage::Type::vertex:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case Shader::Stage::Type::fragment:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        case Shader::Stage::Type::geometry:
            return VK_SHADER_STAGE_GEOMETRY_BIT;
        case Shader::Stage::Type::compute:
            return VK_SHADER_STAGE_COMPUTE_BIT;
    }
    log::panic("Unknown shader type: {}", fmt::underlying(type));
}

VulkanShader::VulkanShader(
  VulkanDevice& device, const Shader::Properties& properties, OptStr name
) : Shader(properties, name), m_device(device) {
    const auto stagesCount = properties.stages.size();
    m_modules.reserve(stagesCount);
    m_pipelineStageInfos.reserve(stagesCount);

    processStages();
    prepareAttributeDescriptions();
    createDescriptorSetLayouts();
}

VulkanShader::~VulkanShader() {
    for (auto& descriptorSetLayout : m_descriptorSetLayouts) {
        if (descriptorSetLayout) {
            log::trace(
              "vkDestroyDescriptorSetLayout: {}",
              static_cast<void*>(descriptorSetLayout)
            );
            vkDestroyDescriptorSetLayout(
              m_device.logical.handle, descriptorSetLayout, m_device.allocator
            );
        }
    }
    for (auto& module : m_modules)
        vkDestroyShaderModule(m_device.logical.handle, module, m_device.allocator);
}

void VulkanShader::prepareAttributeDescriptions() {
    const auto attributeCount = properties.layout.inputAttributes.fields.size();
    m_attributeDescriptions.reserve(attributeCount);

    for (const auto& attribute : properties.layout.inputAttributes.fields) {
        VkVertexInputAttributeDescription attributeDescription;
        attributeDescription.location = attribute.location;
        attributeDescription.binding  = 0;
        attributeDescription.offset   = attribute.offset;
        attributeDescription.format   = toVk(attribute.type);
        m_attributeDescriptions.push_back(attributeDescription);
    }
}

void VulkanShader::createDescriptorSetLayouts() {
    log::debug("Creating descriptor set layouts");
    createDescriptorSetLayout(Uniform::Scope::global);
    createDescriptorSetLayout(Uniform::Scope::local);
}

void VulkanShader::createDescriptorSetLayout(Uniform::Scope scope) {
    log::debug("Creating {} descriptor set layout", scope);

    auto& bindings = m_descriptorSetsBindings.at(static_cast<u8>(scope));

    const auto& setDescription =
      scope == Uniform::Scope::local
        ? properties.layout.localDescriptorSet
        : properties.layout.globalDescriptorSet;

    const auto samplerCount = setDescription.samplers.size();

    VkDescriptorSetLayoutBinding bindingLayout;
    bindingLayout.pImmutableSamplers = nullptr;
    bindingLayout.stageFlags =
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    const auto nonSamplerCount = setDescription.nonSamplers.size();
    log::debug("\tNon-sampler uniform count: {:02}", nonSamplerCount);

    std::vector<VkDescriptorSetLayoutBinding> bindingLayouts;
    bindingLayouts.reserve(nonSamplerCount + 1);

    if (nonSamplerCount > 0) {
        bindingLayout.descriptorCount = 1;
        bindingLayout.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        bindingLayout.binding         = bindings.count;

        bindings.ubo.emplace(bindings.count++);
        bindingLayouts.push_back(bindingLayout);
        log::debug("\tUBO binding: {:02}.", bindingLayout.binding);
    }

    log::debug("\tSampler count: {:02}", samplerCount);

    for (u64 i = 0; i < samplerCount; ++i) {
        bindingLayout.descriptorCount = 1;
        bindingLayout.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindingLayout.binding         = bindings.count;

        bindings.sampler.emplace(bindings.count++);
        bindingLayouts.push_back(bindingLayout);
        log::debug("\tSampler binding: {:02}.", bindingLayout.binding);
    }

    log::trace(
      "Creating descriptor set layout {}. bindings: {}",
      m_descriptorSetLayouts.size() + 1, bindings.count
    );

    VkDescriptorSetLayoutCreateInfo info;
    clearMemory(&info);
    info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    info.bindingCount = bindings.count;
    info.pBindings    = bindingLayouts.data();

    VkDescriptorSetLayout layout;
    log::expect(vkCreateDescriptorSetLayout(
      m_device.logical.handle, &info, m_device.allocator, &layout
    ));
    log::trace("vkCreateDescriptorSetLayout: {}", static_cast<void*>(layout));

    m_descriptorSetLayouts.push_back(layout);
}

void VulkanShader::processStages() {
    for (const auto& stage : properties.stages) processStage(stage);
}

const VulkanShader::PipelineStageInfos& VulkanShader::getPipelineStageInfos() const {
    return m_pipelineStageInfos;
}

const VulkanShader::InputAttributesDescriptions&
  VulkanShader::getInputAttributesDescriptions() const {
    return m_attributeDescriptions;
}

const std::vector<VkDescriptorSetLayout>& VulkanShader::getDescriptorSetLayouts(
) const {
    return m_descriptorSetLayouts;
}

const std::vector<VkShaderModule> VulkanShader::getModules() const {
    return m_modules;
}

const VulkanShader::Bindings& VulkanShader::getDescriptorSetBindings(
  Uniform::Scope scope
) const {
    log::expect(
      scope == Uniform::Scope::local || scope == Uniform::Scope::global,
      "Invalid descriptor set binding scope: {}", fmt::underlying(scope)
    );
    return m_descriptorSetsBindings.at(static_cast<u8>(scope));
}

void VulkanShader::processStage(const Shader::Stage& stage) {
    const auto& code = stage.sourceCode;

    VkShaderModuleCreateInfo moduleCreateInfo;
    clearMemory(&moduleCreateInfo);
    moduleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pCode    = reinterpret_cast<const u32*>(code.data());
    moduleCreateInfo.codeSize = code.size();

    VkShaderModule shaderModule = VK_NULL_HANDLE;

    log::expect(vkCreateShaderModule(
      m_device.logical.handle, &moduleCreateInfo, m_device.allocator, &shaderModule
    ));

    m_modules.push_back(shaderModule);

    VkPipelineShaderStageCreateInfo pipelineStageInfo;
    clearMemory(&pipelineStageInfo);
    pipelineStageInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineStageInfo.stage  = toVk(stage.type);
    pipelineStageInfo.module = shaderModule;
    pipelineStageInfo.pName  = "main";

    m_pipelineStageInfos.push_back(pipelineStageInfo);
}

}  // namespace sl::vk

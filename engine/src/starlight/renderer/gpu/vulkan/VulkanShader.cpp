#include "VulkanShader.hh"

#include <array>
#include <ranges>
#include <unordered_map>

#include "starlight/core/Core.hh"
#include "starlight/core/window/Window.hh"
#include "starlight/renderer/gpu/Texture.hh"

#include "VulkanTexture.hh"
#include "VulkanPipeline.hh"
#include "VulkanDevice.hh"

namespace sl::vk {

constexpr u32 shaderMaxStages           = 8;
constexpr u32 shaderMaxGlobalTextures   = 31;
constexpr u32 shaderMaxInstanceTextures = 31;
constexpr u32 shaderMaxAttributes       = 16;
constexpr u32 descSetIndexGlobal        = 0;
constexpr u32 descSetIndexInstance      = 1;
constexpr u32 bindingIndexUBO           = 0;
constexpr u32 bindingIndexSampler       = 1;

static const std::unordered_map<Shader::Stage::Type, VkShaderStageFlagBits>
  VulkanShaderStages = {
      { Shader::Stage::Type::fragment, VK_SHADER_STAGE_FRAGMENT_BIT },
      { Shader::Stage::Type::geometry, VK_SHADER_STAGE_GEOMETRY_BIT },
      { Shader::Stage::Type::vertex,   VK_SHADER_STAGE_VERTEX_BIT   },
      { Shader::Stage::Type::compute,  VK_SHADER_STAGE_COMPUTE_BIT  }
};

static const std::unordered_map<Shader::Attribute::Type, VkFormat>
  vkAttributeTypes = {
      { Shader::Attribute::Type::float32,   VK_FORMAT_R32_SFLOAT          },
      { Shader::Attribute::Type::float32_2, VK_FORMAT_R32G32_SFLOAT       },
      { Shader::Attribute::Type::float32_3, VK_FORMAT_R32G32B32_SFLOAT    },
      { Shader::Attribute::Type::float32_4, VK_FORMAT_R32G32B32A32_SFLOAT },
      { Shader::Attribute::Type::int8,      VK_FORMAT_R8_SINT             },
      { Shader::Attribute::Type::uint8,     VK_FORMAT_R8_UINT             },
      { Shader::Attribute::Type::int16,     VK_FORMAT_R16_SINT            },
      { Shader::Attribute::Type::uint16,    VK_FORMAT_R16_UINT            },
      { Shader::Attribute::Type::int32,     VK_FORMAT_R32_SINT            },
      { Shader::Attribute::Type::uint32,    VK_FORMAT_R32_UINT            },
};

VkShaderStageFlagBits getStageFlagBits(Shader::Stage::Type type) {
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

VulkanShaderStage::VulkanShaderStage(VulkanDevice& device, const Properties& props) :
    m_device(device), m_handle(VK_NULL_HANDLE) {
    clearMemory(&m_moduleCreateInfo);
    m_moduleCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    m_moduleCreateInfo.codeSize = props.source.size();
    m_moduleCreateInfo.pCode    = (uint32_t*)(props.source.data());

    log::expect(vkCreateShaderModule(
      m_device.logical.handle, &m_moduleCreateInfo, m_device.allocator, &m_handle
    ));

    log::debug("Shader module created");

    clearMemory(&m_stageCreateInfo);
    m_stageCreateInfo.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    m_stageCreateInfo.stage  = getStageFlagBits(props.type);
    m_stageCreateInfo.module = m_handle;
    m_stageCreateInfo.pName  = "main";
}

VulkanShaderStage::~VulkanShaderStage() {
    if (m_handle)
        vkDestroyShaderModule(m_device.logical.handle, m_handle, m_device.allocator);
}

VulkanShader::VulkanShader(VulkanDevice& device, const Shader::Properties& props) :
    Shader(props), m_device(device), m_requiredUboAlignment(0), m_globalUboSize(0),
    m_globalUboStride(0), m_globalUboOffset(0), m_uboSize(0), m_uboStride(0),
    m_pushConstantSize(0), m_pushConstantStride(128), m_instanceTextureCount(0),
    m_boundInstanceId(0), m_boundUboOffset(0), m_pushConstantRangeCount(0),
    m_attributeStride(0), m_maxDescriptorSetCount(0), m_descriptorSetCount(0) {
    addAttributes(props.attributes);
    addUniforms(props.uniformProperties, props.defaultTexture);

    static constexpr int maxStages = 2;

    const auto stageCount = props.stages.size();

    log::expect(
      stageCount <= maxStages, "Stage count {} exceeds maximum {}", stageCount,
      maxStages
    );

    static const u32 maxDescriptorAllocateCount = 1024;

    m_globalUniformCount          = 0;
    m_globalUniformSamplerCount   = 0;
    m_instanceUniformCount        = 0;
    m_instanceUniformSamplerCount = 0;
    m_localUniformCount           = 0;

    for (auto& uniform : m_uniforms | std::views::values) {
        bool isSampler = uniform.isSampler();

        switch (uniform.scope) {
            case Scope::global:
                if (isSampler)
                    m_globalUniformSamplerCount++;
                else
                    m_globalUniformCount++;
                break;
            case Scope::instance:
                if (isSampler)
                    m_instanceUniformSamplerCount++;
                else
                    m_instanceUniformCount++;
                break;
            case Scope::local:
                m_localUniformCount++;
                break;
        }
    }

    log::info(
      "Shader resource - global samplers: {}, global uniforms: {}, instance samplers: {}, instance uniforms: {}, local uniforms: {} ",
      m_globalUniformSamplerCount, m_globalUniformCount,
      m_instanceUniformSamplerCount, m_instanceUniformCount, m_localUniformCount
    );

    m_maxDescriptorSetCount = maxDescriptorAllocateCount;
    m_poolSizes[0] = VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024 };
    m_poolSizes[1] =
      VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4096 };

    // HERE!!

    if (m_globalUniformCount > 0 || m_globalUniformSamplerCount > 0) {
        m_descriptorSets.emplace_back();
        auto& descriptorSetConfig = m_descriptorSets.back();
        auto& bindingIndex        = descriptorSetConfig.bindingCount;

        if (m_globalUniformCount > 0) {
            descriptorSetConfig.bindings[bindingIndex].binding = bindingIndex;
            descriptorSetConfig.bindings[bindingIndex].descriptorCount = 1;
            descriptorSetConfig.bindings[bindingIndex].descriptorType =
              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorSetConfig.bindings[bindingIndex].stageFlags =
              VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            descriptorSetConfig.bindings[bindingIndex].pImmutableSamplers = nullptr;
            bindingIndex++;
        }
        if (m_globalUniformSamplerCount > 0) {
            descriptorSetConfig.bindings[bindingIndex].binding = bindingIndex;
            descriptorSetConfig.bindings[bindingIndex].descriptorCount =
              m_globalUniformSamplerCount;
            descriptorSetConfig.bindings[bindingIndex].descriptorType =
              VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorSetConfig.bindings[bindingIndex].stageFlags =
              VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            descriptorSetConfig.bindings[bindingIndex].pImmutableSamplers = nullptr;
            descriptorSetConfig.samplerBindingIndex = bindingIndex;
            bindingIndex++;
        }

        m_descriptorSetCount++;
    }
    // TODO: unify
    if (m_instanceUniformCount > 0 || m_instanceUniformSamplerCount > 0) {
        m_descriptorSets.emplace_back();
        auto& descriptorSetConfig = m_descriptorSets.back();
        auto& bindingIndex        = descriptorSetConfig.bindingCount;

        if (m_instanceUniformCount > 0) {
            descriptorSetConfig.bindings[bindingIndex].binding = bindingIndex;
            descriptorSetConfig.bindings[bindingIndex].descriptorCount = 1;
            descriptorSetConfig.bindings[bindingIndex].descriptorType =
              VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorSetConfig.bindings[bindingIndex].stageFlags =
              VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            descriptorSetConfig.bindings[bindingIndex].pImmutableSamplers = nullptr;
            bindingIndex++;
        }
        if (m_instanceUniformSamplerCount > 0) {
            descriptorSetConfig.bindings[bindingIndex].binding = bindingIndex;
            descriptorSetConfig.bindings[bindingIndex].descriptorCount =
              m_instanceUniformSamplerCount;
            descriptorSetConfig.bindings[bindingIndex].descriptorType =
              VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorSetConfig.bindings[bindingIndex].stageFlags =
              VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
            descriptorSetConfig.bindings[bindingIndex].pImmutableSamplers = nullptr;
            descriptorSetConfig.samplerBindingIndex = bindingIndex;
            bindingIndex++;
        }
        m_descriptorSetCount++;
    }

    m_descriptorSetLayouts.resize(m_descriptorSetCount);
    m_instanceStates.resize(1024);

    createModules(props.stages);
    processAttributes();
    createDescriptorPool();
    createDescriptorSetLayouts();
    createUniformBuffer();

    log::debug(
      "Shader_{}: global ubo size={} offset={}", getId(), m_globalUboSize,
      m_globalUboOffset
    );
}

VulkanShader::~VulkanShader() {
    const auto allocator     = m_device.allocator;
    const auto logicalDevice = m_device.logical.handle;

    for (int i = 0; i < m_descriptorSetCount; ++i) {
        if (m_descriptorSetLayouts[i]) {
            vkDestroyDescriptorSetLayout(
              logicalDevice, m_descriptorSetLayouts[i], allocator
            );
        }
    }
    if (m_descriptorPool)
        vkDestroyDescriptorPool(logicalDevice, m_descriptorPool, allocator);
    m_uniformBuffer->unlockMemory();
    m_mappedUniformBufferBlock = nullptr;
}

void VulkanShader::addAttributes(std::span<const Shader::Attribute> attributes) {
    m_attributes.reserve(attributes.size());
    for (const auto& attribute : attributes) {
        m_attributeStride += attribute.size;
        m_attributes.push_back(attribute);
    }
}

void VulkanShader::addUniforms(
  std::span<const Shader::Uniform::Properties> uniforms,
  [[maybe_unused]] Texture* defaultTexture
) {
    for (const auto& uniformProps : uniforms) {
        log::expect(
          not m_uniforms.contains(uniformProps.name), "Uniform '{}' already added ",
          uniformProps.name
        );
        if (uniformProps.type == Uniform::Type::sampler) {
            addSampler(uniformProps, defaultTexture);
        } else {
            addUniform(
              uniformProps.name, uniformProps.size, uniformProps.type,
              uniformProps.scope, 0, false
            );
        }
    }
}

void VulkanShader::addSampler(
  const Shader::Uniform::Properties& props, Texture* defaultTexture
) {
    log::expect(
      props.scope != Scope::instance || m_useInstances,
      "Cannot ad instance sampler for shader that doesn't support instances"
    );
    log::expect(
      props.scope != Scope::local, "Samplers cannot be used at local scope"
    );

    u32 location                             = 0;
    static constexpr int maxGlobalTextures   = 128;  // TODO: configurable
    static constexpr int maxInstanceTextures = 128;  // TODO: configurable

    if (props.scope == Scope::global) {
        const auto globalTextureCount = m_globalTextures.size();
        log::expect(
          globalTextureCount + 1 <= maxGlobalTextures,
          "Shader global texture count {} exceed maximum {}", globalTextureCount + 1,
          globalTextureCount
        );
        location = globalTextureCount;
        m_globalTextures.push_back(defaultTexture);
    } else {
        log::expect(
          m_instanceTextureCount + 1 <= maxInstanceTextures,
          "Shader instance texture count {} exceed maximum {}",
          m_instanceTextureCount + 1, maxInstanceTextures
        );
        location = m_instanceTextureCount;
        ++m_instanceTextureCount;
    }
    addUniform(props.name, 0, props.type, props.scope, location, true);
}

void VulkanShader::addUniform(
  const std::string& name, u32 size, Uniform::Type type, Scope scope,
  u32 setLocation, bool isSampler
) {
    static constexpr int maxUniforms = 32;  // TODO: configurable

    const auto uniformCount = m_uniforms.size();
    log::trace("Adding uniform: {}/{}/{}", name, size, scopeToString(scope));

    log::expect(
      uniformCount + 1 <= maxUniforms, "Uniform count {} exceeds limit {}",
      uniformCount + 1, maxUniforms
    );

    Uniform uniform;

    uniform.index    = uniformCount;
    uniform.scope    = scope;
    uniform.type     = type;
    uniform.location = isSampler ? setLocation : uniform.index;

    if (scope != Scope::local) {
        uniform.setIndex = static_cast<u32>(scope);
        uniform.offset =
          isSampler ? 0
          : (scope == Scope::global)
            ? m_globalUboSize
            : m_uboSize;
        uniform.size = isSampler ? 0 : size;
        log::trace("Uniform {} offset: {}", name, uniform.offset);
    } else {
        log::expect(
          scope != Scope::instance || m_useInstances,
          "Cannot add a local uniform for shader that doesn't support locals"
        );

        uniform.setIndex.invalidate();

        const auto r   = Range::aligned(m_pushConstantSize, size, 4);
        uniform.offset = r.offset;
        uniform.size   = r.size;
        m_pushConstantRanges[m_pushConstantRangeCount++] = r;
        m_pushConstantSize += r.size;

        log::trace(
          "Push constant range: {} - {}, total push constants: {}", r.offset, r.size,
          m_pushConstantRangeCount
        );
    }

    m_uniforms[name] = uniform;

    if (not isSampler) {
        if (uniform.scope == Scope::global)
            m_globalUboSize += uniform.size;
        else
            m_uboSize += uniform.size;
    }
}

void VulkanShader::use(CommandBuffer& commandBuffer) {
    m_pipeline->bind(static_cast<VulkanCommandBuffer&>(commandBuffer));
}

void VulkanShader::bindGlobals() {
    // Global UBO is always at the beginning
    m_boundUboOffset = m_globalUboOffset;
}

void VulkanShader::bindInstance(u32 instanceId) {
    m_boundInstanceId = instanceId;
    m_boundUboOffset  = *m_instanceStates[instanceId].offset;
}

void VulkanShader::applyGlobals(CommandBuffer& commandBuffer, u32 imageIndex) {
    // Apply UBO
    VkDescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = m_uniformBuffer->getHandle();
    bufferInfo.offset = m_globalUboOffset;
    bufferInfo.range  = m_globalUboStride;

    // Update desriptor sets
    VkWriteDescriptorSet uboWrite;
    clearMemory(&uboWrite);
    uboWrite.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    uboWrite.dstSet          = m_globalDescriptorSets[imageIndex];
    uboWrite.dstBinding      = 0;
    uboWrite.dstArrayElement = 0;
    uboWrite.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboWrite.descriptorCount = 1;
    uboWrite.pBufferInfo     = &bufferInfo;

    std::array<VkWriteDescriptorSet, 2> descriptorWrites;
    descriptorWrites[0] = uboWrite;

    const auto globalSetBindingCount =
      m_descriptorSets[descSetIndexGlobal].bindingCount;

    std::vector<VkDescriptorImageInfo> imageInfos;
    if (m_globalUniformSamplerCount > 0) {
        imageInfos.reserve(m_globalTextures.size());
        for (u32 i = 0; i < m_globalTextures.size(); ++i) {
            const auto texture =
              static_cast<const VulkanTextureBase*>(m_globalTextures[i]);
            log::expect(
              texture,
              "Could not cast texture to internal type, something went wrong"
            );
            imageInfos.emplace_back(
              texture->getSampler(), texture->getView(),
              VK_IMAGE_LAYOUT_GENERAL  // TODO: transition layout?
            );

            VkWriteDescriptorSet samplerDescriptor;
            clearMemory(&samplerDescriptor);
            samplerDescriptor.sType      = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            samplerDescriptor.dstSet     = m_globalDescriptorSets[imageIndex];
            samplerDescriptor.dstBinding = 1;
            samplerDescriptor.descriptorType =
              VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            samplerDescriptor.descriptorCount = imageInfos.size();
            samplerDescriptor.pImageInfo      = imageInfos.data();

            descriptorWrites[1] = samplerDescriptor;
        }
    }

    vkUpdateDescriptorSets(
      m_device.logical.handle, globalSetBindingCount, descriptorWrites.data(), 0, 0
    );

    auto globalDescriptor = &m_globalDescriptorSets[imageIndex];

    vkCmdBindDescriptorSets(
      static_cast<VulkanCommandBuffer&>(commandBuffer).getHandle(),
      VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->getLayout(), 0, 1,
      globalDescriptor, 0, 0
    );
}

void VulkanShader::applyInstance(CommandBuffer& commandBuffer, u32 imageIndex) {
    auto objectState = &m_instanceStates[m_boundInstanceId];
    auto objectDescriptorSet =
      objectState->descriptorSetState.descriptorSets[imageIndex];

    std::array<VkWriteDescriptorSet, 2> descriptorWrites;
    VkDescriptorBufferInfo bufferInfo;

    u32 descriptorCount = 0;
    u32 descriptorIndex = 0;

    // 0 - uniform buffer
    if (m_instanceUniformCount > 0) {
        auto& instanceUboGeneration =
          objectState->descriptorSetState.descriptorStates[descriptorIndex]
            .generations[imageIndex];

        if (not instanceUboGeneration.hasValue()) {
            bufferInfo.buffer = m_uniformBuffer->getHandle();
            bufferInfo.offset = *objectState->offset;
            bufferInfo.range  = m_uboStride;

            VkWriteDescriptorSet uboDescriptor;
            clearMemory(&uboDescriptor);
            uboDescriptor.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            uboDescriptor.dstSet          = objectDescriptorSet;
            uboDescriptor.dstBinding      = descriptorIndex;
            uboDescriptor.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uboDescriptor.pBufferInfo     = &bufferInfo;
            uboDescriptor.descriptorCount = 1;

            descriptorWrites[descriptorCount++] = uboDescriptor;
            instanceUboGeneration               = 1;
        }
        descriptorIndex++;
    }

    std::vector<VkDescriptorImageInfo> imageInfos;
    if (m_instanceUniformSamplerCount > 0) {
        // iterate samplers
        const u8 bindingIndex =
          *m_descriptorSets[descSetIndexInstance].samplerBindingIndex;
        auto totalSamplerCount =
          m_descriptorSets[descSetIndexInstance]
            .bindings[bindingIndex]
            .descriptorCount;
        imageInfos.reserve(totalSamplerCount);
        for (u32 i = 0; i < totalSamplerCount; ++i) {
            const auto texture = static_cast<const VulkanTextureBase*>(
              m_instanceStates[m_boundInstanceId].instanceTextures[i]
            );
            log::expect(
              texture,
              "Could not cast texture to internal type, something went wrong"
            );
            imageInfos.emplace_back(
              texture->getSampler(), texture->getView(),
              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            );
        }

        VkWriteDescriptorSet samplerDescriptor;
        clearMemory(&samplerDescriptor);
        samplerDescriptor.sType          = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        samplerDescriptor.dstSet         = objectDescriptorSet;
        samplerDescriptor.dstBinding     = descriptorIndex;
        samplerDescriptor.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerDescriptor.descriptorCount = totalSamplerCount;
        samplerDescriptor.pImageInfo      = imageInfos.data();

        descriptorWrites[descriptorCount++] = samplerDescriptor;
    }

    if (descriptorCount > 0) {
        vkUpdateDescriptorSets(
          m_device.logical.handle, descriptorCount, descriptorWrites.data(), 0, 0
        );
    }

    vkCmdBindDescriptorSets(
      static_cast<VulkanCommandBuffer&>(commandBuffer).getHandle(),
      VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline->getLayout(), 1, 1,
      &objectDescriptorSet, 0, 0
    );
}

u32 VulkanShader::acquireInstanceResources(const std::vector<Texture*>& textures) {
    Id32 id = 0;

    for (int i = 0; i < 1024; ++i) {
        if (not m_instanceStates[i].id.hasValue()) {
            id                     = i;
            m_instanceStates[i].id = i;
            break;
        }
    }
    log::expect(id.hasValue(), "Could not acquire new resource id");

    auto& instanceState = m_instanceStates[*id];
    const u8 bindingIndex =
      *m_descriptorSets[descSetIndexInstance].samplerBindingIndex;
    [[maybe_unused]] const auto instanceTextureCount =
      m_descriptorSets[descSetIndexInstance].bindings[bindingIndex].descriptorCount;

    if (textures.size() != m_instanceTextureCount) {
        log::warn(
          "Provided texture map range size doesn't match with instance texture count, ignoring and setting all to default map "
        );
        instanceState.instanceTextures.resize(
          m_instanceTextureCount, nullptr  // TODO: pass default texture
        );
    } else {
        instanceState.instanceTextures.reserve(m_instanceTextureCount);
        for (auto& map : textures) instanceState.instanceTextures.push_back(map);
    }

    // todo: should we set all to default?

    // allocate space in the UBO - by the stride, not the size
    if (m_uboStride > 0) {
        instanceState.offset = m_uniformBuffer->allocate(m_uboStride)->offset;
        log::info(
          "Shader {} allocated offset={} for instance resources", m_name,
          instanceState.offset.get()
        );
    } else {
        log::info("UBO stride equals 0, not allocating memory");
    }

    auto& setState = instanceState.descriptorSetState;
    [[maybe_unused]] const auto bindingCount =
      m_descriptorSets[descSetIndexInstance].bindingCount;
    setState.descriptorStates.resize(maxBindings);

    // allocate 3 descirptor sets, one per frame
    std::array<VkDescriptorSetLayout, 3> layouts = {
        m_descriptorSetLayouts[descSetIndexInstance],
        m_descriptorSetLayouts[descSetIndexInstance],
        m_descriptorSetLayouts[descSetIndexInstance]
    };

    VkDescriptorSetAllocateInfo allocateInfo;
    clearMemory(&allocateInfo);
    allocateInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool     = m_descriptorPool;
    allocateInfo.descriptorSetCount = 3;
    allocateInfo.pSetLayouts        = layouts.data();

    log::expect(vkAllocateDescriptorSets(
      m_device.logical.handle, &allocateInfo,
      instanceState.descriptorSetState.descriptorSets.data()
    ));

    return *id;
}

void VulkanShader::releaseInstanceResources(u32 instanceId) {
    log::trace(
      "Releasing instance ({}) resources for Shader ({}): ", instanceId, getId()
    );
    const auto logicalDevice = m_device.logical.handle;
    vkDeviceWaitIdle(logicalDevice);

    auto& instanceState = m_instanceStates[instanceId];

    log::expect(vkFreeDescriptorSets(
      logicalDevice, m_descriptorPool, 3,
      instanceState.descriptorSetState.descriptorSets.data()
    ));

    // TODO: clear in place instead
    instanceState.descriptorSetState.descriptorStates.clear();
    instanceState.descriptorSetState.descriptorStates.resize(maxBindings);
    instanceState.instanceTextures.clear();

    if (instanceState.offset.hasValue()) {
        m_uniformBuffer->free(Range{
          .offset = *instanceState.offset,
          .size   = m_uboStride,
        });
    }
    instanceState.id.invalidate();
    instanceState.offset.invalidate();
}

void VulkanShader::setSampler(const std::string& name, const Texture* value) {
    if (auto& uniform = m_uniforms[name]; uniform.scope == Scope::global) {
        m_globalTextures[uniform.location] = value;
    } else {
        m_instanceStates[m_boundInstanceId].instanceTextures[uniform.location] =
          value;
    }
}

void VulkanShader::setUniform(
  const std::string& name, const void* value, CommandBuffer& commandBuffer
) {
    if (auto& uniform = m_uniforms[name]; uniform.scope == Scope::local) {
        vkCmdPushConstants(
          static_cast<VulkanCommandBuffer&>(commandBuffer).getHandle(),
          m_pipeline->getLayout(),
          VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, uniform.offset,
          uniform.size, value
        );
    } else {
        const auto offset = m_boundUboOffset + uniform.offset;
        void* address     = static_cast<void*>(
          static_cast<char*>(m_mappedUniformBufferBlock) + offset
        );
        std::memcpy(address, value, uniform.size);
    }
}

void VulkanShader::createModules(std::span<const Stage> stages) {
    m_stages.reserve(stages.size());
    for (const auto& stageConfig : stages) {
        m_stages.emplace_back(
          m_device,
          VulkanShaderStage::Properties{ stageConfig.source, stageConfig.type }
        );
    }
}

void VulkanShader::createDescriptorPool() {
    VkDescriptorPoolCreateInfo poolInfo;
    poolInfo.pNext         = nullptr;
    poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 2;
    poolInfo.pPoolSizes    = m_poolSizes.data();
    poolInfo.maxSets       = m_maxDescriptorSetCount;
    poolInfo.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    log::expect(vkCreateDescriptorPool(
      m_device.logical.handle, &poolInfo, m_device.allocator, &m_descriptorPool
    ));
}

void VulkanShader::createDescriptorSetLayouts() {
    for (int i = 0; i < m_descriptorSetCount; ++i) {
        log::trace(
          "Creating descriptor set layout: {} - bindings: {}", i,
          m_descriptorSets[i].bindingCount
        );

        VkDescriptorSetLayoutCreateInfo info;
        clearMemory(&info);
        info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        info.bindingCount = m_descriptorSets[i].bindingCount;
        info.pBindings    = m_descriptorSets[i].bindings.data();

        log::expect(vkCreateDescriptorSetLayout(
          m_device.logical.handle, &info, m_device.allocator,
          &m_descriptorSetLayouts[i]
        ));
    }
}

static std::unordered_map<PolygonMode, VkPolygonMode> vkPolygonModes = {
    { PolygonMode::fill,  VK_POLYGON_MODE_FILL  },
    { PolygonMode::line,  VK_POLYGON_MODE_LINE  },
    { PolygonMode::point, VK_POLYGON_MODE_POINT },
};

void VulkanShader::bindPipeline(Pipeline& pipeline) {
    m_pipeline = static_cast<VulkanPipeline*>(&pipeline);
}

VulkanPipeline::Properties VulkanShader::getPipelineProperties() {
    const auto size = Window::get().getSize();

    VkViewport viewport;
    viewport.x        = 0.0f;
    viewport.y        = static_cast<float>(size.h);
    viewport.width    = static_cast<float>(size.w);
    viewport.height   = -static_cast<float>(size.h);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor;
    scissor.offset.x = scissor.offset.y = 0;
    scissor.extent.width                = size.w;
    scissor.extent.height               = size.h;

    std::vector<VkPipelineShaderStageCreateInfo> stageCreateInfos;
    stageCreateInfos.reserve(m_stages.size());
    for (auto& stage : m_stages)
        stageCreateInfos.push_back(stage.getStageCreateInfo());

    VulkanPipeline::Properties pipelineProps;
    pipelineProps.stride               = m_attributeStride;
    pipelineProps.vertexAttributes     = m_attributeDescriptions;
    pipelineProps.descriptorSetLayouts = m_descriptorSetLayouts;
    pipelineProps.stages               = stageCreateInfos;
    pipelineProps.viewport             = viewport;
    pipelineProps.scissor              = scissor;
    pipelineProps.polygonMode          = vkPolygonModes.at(m_polygonMode);
    pipelineProps.depthTestEnabled     = true;
    pipelineProps.cullMode             = m_cullMode;

    // todo: use vector

    pipelineProps.pushConstantRanges = {
        m_pushConstantRanges.data(),
        m_pushConstantRanges.data() + m_pushConstantRangeCount
    };

    return pipelineProps;
}

void VulkanShader::createUniformBuffer() {
    m_requiredUboAlignment =
      m_device.physical.info.coreProperties.limits.minUniformBufferOffsetAlignment;
    m_globalUboStride = getAlignedValue(m_globalUboSize, m_requiredUboAlignment);
    m_uboStride       = getAlignedValue(m_uboSize, m_requiredUboAlignment);

    log::info("Minimal uniform buffer offset alignment={}", m_requiredUboAlignment);

    const auto deviceLocalBits =
      m_device.physical.info.supportsDeviceLocalHostVisibleMemory
        ? MemoryProperty::MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        : MemoryProperty::undefined;

    // material max count should be configurable!
    static constexpr u64 maxMaterials = 1024;
    const u64 totalBufferSize = m_globalUboStride + (m_uboStride * maxMaterials);

    Buffer::Properties bufferProps;
    bufferProps.size = totalBufferSize;
    bufferProps.memoryProperty =
      deviceLocalBits | MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT;

    bufferProps.usage =
      BufferUsage::BUFFER_USAGE_TRANSFER_DST_BIT
      | BufferUsage::BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    bufferProps.bindOnCreate = true;

    log::debug(
      "Creating uniform buffer, globalUboStride={}, uboStride={}, totalSize={}",
      m_globalUboStride, m_uboStride, totalBufferSize
    );
    m_uniformBuffer.emplace(m_device, bufferProps);

    log::debug("Allocating {}b of memory", m_globalUboStride);
    m_globalUboOffset = m_uniformBuffer->allocate(m_globalUboStride)->offset;
    m_mappedUniformBufferBlock = m_uniformBuffer->lockMemory();

    std::array<VkDescriptorSetLayout, 3> globalLayouts = {
        m_descriptorSetLayouts[descSetIndexGlobal],
        m_descriptorSetLayouts[descSetIndexGlobal],
        m_descriptorSetLayouts[descSetIndexGlobal]
    };
    VkDescriptorSetAllocateInfo allocateInfo;
    clearMemory(&allocateInfo);
    allocateInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocateInfo.descriptorPool     = m_descriptorPool;
    allocateInfo.descriptorSetCount = 3;
    allocateInfo.pSetLayouts        = globalLayouts.data();

    log::expect(vkAllocateDescriptorSets(
      m_device.logical.handle, &allocateInfo, m_globalDescriptorSets.data()
    ));
}

void VulkanShader::processAttributes() {
    u32 offset                = 0;
    const auto attributeCount = m_attributes.size();
    m_attributeDescriptions.reserve(attributeCount);

    for (u32 i = 0; i < attributeCount; ++i) {
        VkVertexInputAttributeDescription attribute;
        attribute.location = i;
        attribute.binding  = 0;
        attribute.offset   = offset;
        attribute.format   = vkAttributeTypes.at(m_attributes[i].type);

        m_attributeDescriptions.push_back(attribute);

        offset += m_attributes[i].size;
    }
}

}  // namespace sl::vk
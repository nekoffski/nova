#include "VulkanShaderDataBinder.hh"

#include "VulkanShader.hh"
#include "VulkanDevice.hh"
#include "VulkanPipeline.hh"
#include "VulkanCommandBuffer.hh"
#include "VulkanBuffer.hh"

namespace sl::vk {

VulkanShaderDataBinder::VulkanShaderDataBinder(
  VulkanDevice& device, VulkanShader& shader
) :
    ShaderDataBinder(shader), m_device(device), m_shader(shader),
    m_dataLayout(shader.properties.layout), m_descriptorPool(VK_NULL_HANDLE),
    m_localDescriptorDirtyFrames(0u), m_globalDescriptorDirtyFrames(0u),
    m_globalUboStride(0u), m_localUboStride(0u), m_globalUboOffset(0u),
    m_uniformBufferView(nullptr),
    m_globalDescriptorSets(maxFramesInFlight, VK_NULL_HANDLE),
    m_globalTextures(m_dataLayout.globalDescriptorSet.samplers.size(), nullptr) {
    createDescriptorPool();
    createUniformBuffer();
}

VulkanShaderDataBinder::~VulkanShaderDataBinder() {
    m_device.waitIdle();

    log::expect(vkFreeDescriptorSets(
      m_device.logical.handle, m_descriptorPool, maxFramesInFlight,
      m_globalDescriptorSets.data()
    ));

    for (auto& localSet : m_localDescriptorSets) {
        if (localSet) {
            log::expect(vkFreeDescriptorSets(
              m_device.logical.handle, m_descriptorPool, maxFramesInFlight,
              localSet->descriptorSets.data()
            ));
        }
    }
    vkDestroyDescriptorPool(
      m_device.logical.handle, m_descriptorPool, m_device.allocator
    );
}

u32 VulkanShaderDataBinder::acquireLocalDescriptorSet() {
    auto localSet       = findFreeLocalDescriptorSet();
    const auto samplers = m_dataLayout.localDescriptorSet.samplers.size();
    localSet->textures.resize(samplers, nullptr);

    if (auto localUboSize = m_dataLayout.localDescriptorSet.nonSamplers.size();
        localUboSize > 0u) {
        auto allocatedRange = m_uniformBuffer->allocate(localUboSize);
        log::expect(
          allocatedRange.has_value(), "Could not allocate space for local UBO"
        );

        localSet->offset = allocatedRange->offset;
        log::debug("Allocated offset={} for instance resources", localSet->offset);
    } else {
        log::debug("No uniforms in local UBO, not allocating memory");
    }

    const auto& bindings =
      m_shader.getDescriptorSetBindings(Shader::Uniform::Scope::local);

    if (bindings.count > 0) {
        auto descriptorSetLayouts = m_shader.getDescriptorSetLayouts();

        std::vector<VkDescriptorSetLayout> localLayouts(
          maxFramesInFlight, descriptorSetLayouts[Shader::uboLocalSet]
        );

        VkDescriptorSetAllocateInfo allocateInfo;
        clearMemory(&allocateInfo);
        allocateInfo.sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocateInfo.descriptorPool = m_descriptorPool;
        allocateInfo.descriptorSetCount = maxFramesInFlight;
        allocateInfo.pSetLayouts        = localLayouts.data();

        log::expect(vkAllocateDescriptorSets(
          m_device.logical.handle, &allocateInfo, localSet->descriptorSets.data()
        ));
        log::trace("vkAllocateDescriptorSets");
        for (auto& descriptorSet : localSet->descriptorSets)
            log::trace("\t {}", static_cast<void*>(descriptorSet));
    }
    return localSet->id;
}

void VulkanShaderDataBinder::releaseLocalDescriptorSet(u32 id) {
    auto& localSet = m_localDescriptorSets[id];

    log::debug("Releaseing local descriptor set: {}", id);
    log::expect(localSet, "Local descriptor set with id {} not found", id);

    m_device.waitIdle();

    log::trace("vkFreeDescriptorSets");
    for (auto& descriptorSet : localSet->descriptorSets)
        log::trace("\t {}", static_cast<void*>(descriptorSet));

    log::expect(vkFreeDescriptorSets(
      m_device.logical.handle, m_descriptorPool, maxFramesInFlight,
      localSet->descriptorSets.data()
    ));

    m_uniformBuffer->free(Range{
      .offset = localSet->offset,
      .size   = m_localUboStride,
    });

    localSet.clear();
}

void VulkanShaderDataBinder::bindDescriptorSet(
  CommandBuffer& commandBuffer, Pipeline& pipeline, VkDescriptorSet& descriptorSet,
  u64 uniformBufferOffset, u64 stride, std::span<const VulkanTexture*> textures,
  u64 nonSamplerCount, u64 descriptorIndex, u8& counter
) {
    if (counter > 0) {
        counter--;

        std::vector<VkWriteDescriptorSet> descriptorWrites;
        VkDescriptorBufferInfo bufferInfo;

        if (nonSamplerCount > 0u) {
            bufferInfo.buffer = m_uniformBuffer->getHandle();
            bufferInfo.offset = uniformBufferOffset;
            bufferInfo.range  = stride;

            VkWriteDescriptorSet uboDescriptor;
            clearMemory(&uboDescriptor);
            uboDescriptor.sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            uboDescriptor.dstSet          = descriptorSet;
            uboDescriptor.dstBinding      = descriptorWrites.size();
            uboDescriptor.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            uboDescriptor.pBufferInfo     = &bufferInfo;
            uboDescriptor.descriptorCount = 1;

            descriptorWrites.push_back(uboDescriptor);
        }

        std::vector<VkDescriptorImageInfo> imageInfos;
        imageInfos.reserve(textures.size());

        for (const auto& texture : textures) {
            imageInfos.emplace_back(
              texture->getSampler(), texture->getView(), texture->getLayout()
            );

            VkWriteDescriptorSet samplerDescriptor;
            clearMemory(&samplerDescriptor);
            samplerDescriptor.sType      = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            samplerDescriptor.dstSet     = descriptorSet;
            samplerDescriptor.dstBinding = descriptorWrites.size();
            samplerDescriptor.descriptorType =
              VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            samplerDescriptor.descriptorCount = 1;
            samplerDescriptor.pImageInfo      = &imageInfos.back();

            descriptorWrites.push_back(samplerDescriptor);
        }

        if (descriptorWrites.size() > 0) {
            vkUpdateDescriptorSets(
              m_device.logical.handle, descriptorWrites.size(),
              descriptorWrites.data(), 0, 0
            );
        }
    }
    vkCmdBindDescriptorSets(
      static_cast<VulkanCommandBuffer&>(commandBuffer).getHandle(),
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      static_cast<VulkanPipeline&>(pipeline).getLayout(), descriptorIndex, 1,
      &descriptorSet, 0, 0
    );
}

void VulkanShaderDataBinder::bindGlobalDescriptorSet(
  CommandBuffer& commandBuffer, u32 imageIndex, Pipeline& pipeline, bool update
) {
    const auto nonSamplerCount = m_dataLayout.globalDescriptorSet.nonSamplers.size();

    if (update) m_globalDescriptorDirtyFrames = maxFramesInFlight;

    bindDescriptorSet(
      commandBuffer, pipeline, m_globalDescriptorSets[imageIndex], m_globalUboOffset,
      m_globalUboStride, m_globalTextures, nonSamplerCount, Shader::uboGlobalSet,
      m_globalDescriptorDirtyFrames
    );
}

void VulkanShaderDataBinder::bindLocalDescriptorSet(
  CommandBuffer& commandBuffer, u32 id, u32 imageIndex, Pipeline& pipeline,
  bool update
) {
    const auto localDescriptor = m_localDescriptorSets[id].get();
    const auto nonSamplerCount = m_dataLayout.localDescriptorSet.nonSamplers.size();

    if (update) m_localDescriptorDirtyFrames = maxFramesInFlight;

    bindDescriptorSet(
      commandBuffer, pipeline, localDescriptor->descriptorSets[imageIndex],
      localDescriptor->offset, m_localUboStride, localDescriptor->textures,
      nonSamplerCount, Shader::uboLocalSet, m_localDescriptorDirtyFrames
    );
}

bool VulkanShaderDataBinder::setGlobalUniform(
  const Shader::Uniform& uniform, const void* value
) {
    return setUniform(
      Range{
        .offset = m_globalUboOffset + uniform.offset,
        .size   = uniform.size,
      },
      value
    );
}

bool VulkanShaderDataBinder::setLocalUniform(
  const Shader::Uniform& uniform, u32 id, const void* value
) {
    return setUniform(
      Range{
        .offset = m_localDescriptorSets[id]->offset + uniform.offset,
        .size   = uniform.size,
      },
      value
    );
}

bool VulkanShaderDataBinder::setUniform(const Range& range, const void* value) {
    void* address =
      static_cast<void*>(static_cast<char*>(m_uniformBufferView) + range.offset);

    if (std::memcmp(address, value, range.size) == 0) [[likely]]
        return false;

    std::memcpy(address, value, range.size);
    return true;
}

bool VulkanShaderDataBinder::setGlobalSampler(
  const Shader::Uniform& uniform, const Texture* value
) {
    return compareAssign(
      m_globalTextures[uniform.offset], static_cast<const VulkanTexture*>(value)
    );
}

bool VulkanShaderDataBinder::setLocalSampler(
  const Shader::Uniform& uniform, u32 id, const Texture* value
) {
    return compareAssign(
      m_localDescriptorSets[id]->textures[uniform.offset],
      static_cast<const VulkanTexture*>(value)
    );
}

void VulkanShaderDataBinder::setPushConstant(
  const Shader::Uniform& uniform, const void* value, CommandBuffer& commandBuffer,
  Pipeline& pipeline
) {
    vkCmdPushConstants(
      static_cast<VulkanCommandBuffer&>(commandBuffer).getHandle(),
      static_cast<VulkanPipeline&>(pipeline).getLayout(),
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, uniform.offset,
      uniform.size, value
    );
}

void VulkanShaderDataBinder::createDescriptorPool() {
    VkDescriptorPoolCreateInfo poolInfo;
    clearMemory(&poolInfo);

    static std::vector<VkDescriptorPoolSize> poolSizes = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1024u },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4096u },
    };
    static constexpr u32 maxDescriptorAllocateCount = 1024u;

    poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes    = poolSizes.data();
    poolInfo.maxSets       = maxDescriptorAllocateCount;
    poolInfo.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    log::expect(vkCreateDescriptorPool(
      m_device.logical.handle, &poolInfo, m_device.allocator, &m_descriptorPool
    ));
    log::trace("vkCreateDescriptorPool: {}", static_cast<void*>(m_descriptorPool));
}

void VulkanShaderDataBinder::createUniformBuffer() {
    const auto requiredUboAlignment =
      m_device.physical.info.coreProperties.limits.minUniformBufferOffsetAlignment;

    m_globalUboStride =
      getAlignedValue(m_dataLayout.globalDescriptorSet.size, requiredUboAlignment);
    m_localUboStride =
      getAlignedValue(m_dataLayout.localDescriptorSet.size, requiredUboAlignment);

    log::debug("Minimal uniform buffer offset alignment: {}", requiredUboAlignment);

    const auto deviceLocalBits =
      m_device.physical.info.supportsDeviceLocalHostVisibleMemory
        ? MemoryProperty::MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        : MemoryProperty::undefined;

    const auto totalBufferSize =
      m_globalUboStride + (m_localUboStride * maxLocalDescriptorSets);

    Buffer::Properties bufferProps{
        .size = totalBufferSize,
        .memoryProperty =
          deviceLocalBits | MemoryProperty::MEMORY_PROPERTY_HOST_VISIBLE_BIT,
        .usage = BufferUsage::BUFFER_USAGE_TRANSFER_DST_BIT
                 | BufferUsage::BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        .bindOnCreate = true,
    };

    log::debug(
      "Creating uniform buffer, globalUboStride={}, localUboStride={}, totalSize = {}",
      m_globalUboStride, m_localUboStride, totalBufferSize
    );
    m_uniformBuffer.emplace(m_device, bufferProps);

    log::debug("Allocating {}b of memory", m_globalUboStride);
    auto allocatedRange = m_uniformBuffer->allocate(m_globalUboStride);
    log::expect(
      allocatedRange.has_value(), "Could not allocate global UBO from uniform buffer"
    );
    m_globalUboOffset   = allocatedRange->offset;
    m_uniformBufferView = m_uniformBuffer->lockMemory();

    const auto& bindings =
      m_shader.getDescriptorSetBindings(Shader::Uniform::Scope::global);

    if (bindings.count > 0) {
        auto descriptorSetLayouts = m_shader.getDescriptorSetLayouts();

        std::vector<VkDescriptorSetLayout> globalLayouts(
          maxFramesInFlight, descriptorSetLayouts[Shader::uboGlobalSet]
        );

        VkDescriptorSetAllocateInfo allocateInfo;
        clearMemory(&allocateInfo);
        allocateInfo.sType          = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocateInfo.descriptorPool = m_descriptorPool;
        allocateInfo.descriptorSetCount = maxFramesInFlight;
        allocateInfo.pSetLayouts        = globalLayouts.data();

        log::expect(vkAllocateDescriptorSets(
          m_device.logical.handle, &allocateInfo, m_globalDescriptorSets.data()
        ));
        log::trace("vkAllocateDescriptorSets");
        for (auto& descriptorSet : m_globalDescriptorSets)
            log::trace("\t {}", static_cast<void*>(descriptorSet));
    }
}

VulkanShaderDataBinder::LocalDescriptorSet*
  VulkanShaderDataBinder::findFreeLocalDescriptorSet() {
    const auto localSamplerCount = m_dataLayout.localDescriptorSet.samplers.size();
    for (u64 i = 0u; i < maxLocalDescriptorSets; ++i)
        if (auto& slot = m_localDescriptorSets[i]; not slot)
            return slot.emplace(i, localSamplerCount);
    log::panic("Could not find free local descriptor set");
}

VulkanShaderDataBinder::LocalDescriptorSet::LocalDescriptorSet(
  u32 id, u32 textureCount
) :
    id(id), offset(0u), descriptorSets({ VK_NULL_HANDLE }),
    textures(textureCount, nullptr) {}

}  // namespace sl::vk

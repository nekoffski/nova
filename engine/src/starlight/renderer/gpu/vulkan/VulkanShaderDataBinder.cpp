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
    m_descriptorPool(VK_NULL_HANDLE), m_globalUboStride(0u), m_localUboStride(0u),
    m_globalUboOffset(0u), m_uniformBufferView(nullptr),
    m_globalDescriptorSets(maxFramesInFlight, VK_NULL_HANDLE) {
    createDescriptorPool();
    createUniformBuffer();
}

u32 VulkanShaderDataBinder::acquireLocalDescriptor() { return u32(); }

void VulkanShaderDataBinder::releaseLocalDescriptor(u32 id) {}

void VulkanShaderDataBinder::bindLocalDescriptor(u32 id) {}

void VulkanShaderDataBinder::bindGlobalDescriptor() {}

void VulkanShaderDataBinder::updateGlobalDescriptor(
  CommandBuffer& commandBuffer, u32 imageIndex, Pipeline& pipeline
) {}

void VulkanShaderDataBinder::updateLocalDescriptor(
  CommandBuffer& commandBuffer, u32 imageIndex, Pipeline& pipeline
) {}

void VulkanShaderDataBinder::setGlobalUniform(
  const Shader::Uniform& uniform, const void* value
) {}

void VulkanShaderDataBinder::setGlobalSampler(
  const Shader::Uniform& uniform, const Texture* value
) {}

void VulkanShaderDataBinder::setLocalSampler(
  const Shader::Uniform& uniform, const Texture* value
) {}

void VulkanShaderDataBinder::setLocalUniform(
  const Shader::Uniform& uniform, const void* value
) {}

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
      getAlignedValue(m_shader.getGlobalUboSize(), requiredUboAlignment);
    m_localUboStride =
      getAlignedValue(m_shader.getLocalUboSize(), requiredUboAlignment);

    log::debug("Minimal uniform buffer offset alignment: {}", requiredUboAlignment);

    const auto deviceLocalBits =
      m_device.physical.info.supportsDeviceLocalHostVisibleMemory
        ? MemoryProperty::MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        : MemoryProperty::undefined;

    // TODO: read from config
    static constexpr u64 maxLocals = 1024;
    const auto totalBufferSize = m_globalUboStride + (m_localUboStride * maxLocals);

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
    m_globalUboOffset = allocatedRange->offset;

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

}  // namespace sl::vk

#include "ShaderDataBinder.hh"

#include "Device.hh"

#ifdef SL_USE_VK
#include "vulkan/VulkanDevice.hh"
#include "vulkan/VulkanShaderDataBinder.hh"
#include "vulkan/VulkanShader.hh"
#endif

namespace sl {

/*
    ShaderDataBinder::Setter
*/

ShaderDataBinder::Setter::Setter(
  UniformSetter&& uniformSetter, SamplerSetter&& samplerSetter,
  const Shader::DataLayout::DescriptorSet& descriptorLayout
) :
    m_updated(false), m_uniformSetter(std::forward<UniformSetter>(uniformSetter)),
    m_samplerSetter(std::forward<SamplerSetter>(samplerSetter)),
    m_descriptorLayout(descriptorLayout) {}

void ShaderDataBinder::Setter::set(
  const std::string& uniform, const Texture* value
) {
    static constexpr bool isSampler = true;
    m_updated |= m_samplerSetter(getUniform(uniform, isSampler), value);
}

bool ShaderDataBinder::Setter::wasUpdated() const { return m_updated; }

const Shader::Uniform& ShaderDataBinder::Setter::getUniform(
  const std::string& uniform, bool isSampler
) const {
    const auto& container =
      isSampler ? m_descriptorLayout.samplers : m_descriptorLayout.nonSamplers;
    log::expect(container.contains(uniform), "Could not find '{}' uniform", uniform);
    return container.at(uniform);
}

/*
    ShaderDataBinder
*/

UniquePtr<ShaderDataBinder> ShaderDataBinder::create(Shader& shader) {
#ifdef SL_USE_VK
    return UniquePtr<vk::VulkanShaderDataBinder>::create(
      static_cast<vk::VulkanDevice&>(Device::get().getImpl()),
      static_cast<vk::VulkanShader&>(shader)
    );
#else
    log::panic("GPU API provider not specified");
#endif
}

ShaderDataBinder::ShaderDataBinder(Shader& shader
) : m_dataLayout(shader.properties.layout) {}

void ShaderDataBinder::setGlobalUniforms(
  Pipeline& pipeline, CommandBuffer& commandBuffer, u64 frameNumber, u32 imageIndex,
  UniformCallback&& callback
) {
    Setter globalSetter{
        [&](const auto& uniform, const void* value) -> bool {
            return setGlobalUniform(uniform, value);
        },
        [&](const auto& uniform, const Texture* value) -> bool {
            return setGlobalSampler(uniform, value);
        },
        m_dataLayout.globalDescriptorSet
    };
    callback(globalSetter);

    bindGlobalDescriptorSet(
      commandBuffer, frameNumber, imageIndex, pipeline, globalSetter.wasUpdated()
    );
}

void ShaderDataBinder::setLocalUniforms(
  Pipeline& pipeline, CommandBuffer& commandBuffer, u64 frameNumber, u32 id,
  u32 imageIndex, UniformCallback&& callback
) {
    Setter localSetter{
        [&](const auto& uniform, const void* value) -> bool {
            return setLocalUniform(uniform, id, value);
        },
        [&](const auto& uniform, const Texture* value) -> bool {
            return setLocalSampler(uniform, id, value);
        },
        m_dataLayout.localDescriptorSet
    };
    callback(localSetter);

    bindLocalDescriptorSet(
      commandBuffer, frameNumber, id, imageIndex, pipeline, localSetter.wasUpdated()
    );
}

}  // namespace sl

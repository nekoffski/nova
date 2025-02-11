#include "ShaderDataBinder.hh"

namespace sl {

/*
    ShaderDataBinder::Setter
*/

ShaderDataBinder::Setter::Setter(
  UniformSetter&& uniformSetter, SamplerSetter&& samplerSetter,
  const Shader::Uniforms::UniformsMap& uniforms
) :
    m_uniformSetter(std::forward<UniformSetter>(uniformSetter)),
    m_samplerSetter(std::forward<SamplerSetter>(samplerSetter)),
    m_uniforms(uniforms) {}

void ShaderDataBinder::Setter::set(
  const std::string& uniform, const Texture* value
) {
    m_samplerSetter(getUniform(uniform), value);
}

const Shader::Uniform& ShaderDataBinder::Setter::getUniform(
  const std::string& uniform
) const {
    log::expect(
      m_uniforms.contains(uniform), "Could not find '{}' uniform", uniform
    );
    return *m_uniforms.at(uniform);
}

/*
    ShaderDataBinder
*/

ShaderDataBinder::ShaderDataBinder(Shader& shader) :
    m_uniforms(shader.getUniforms()),
    m_globalSetter(
      [&](const auto& uniform, const void* value) {
          setGlobalUniform(uniform, value);
      },
      [&](const auto& uniform, const Texture* value) {
          setGlobalSampler(uniform, value);
      },
      m_uniforms.get(Shader::Uniform::Scope::global)
    ) {}

void ShaderDataBinder::setGlobalUniforms(
  Pipeline& pipeline, CommandBuffer& commandBuffer, u32 imageIndex,
  UniformCallback&& callback
) {
    callback(m_globalSetter);
    updateGlobalDescriptorSet(commandBuffer, imageIndex, pipeline);
}

void ShaderDataBinder::setLocalUniforms(
  Pipeline& pipeline, CommandBuffer& commandBuffer, u32 id, u32 imageIndex,
  UniformCallback&& callback
) {
    Setter localSetter{
        [&](const auto& uniform, const void* value) {
            setLocalUniform(uniform, id, value);
        },
        [&](const auto& uniform, const Texture* value) {
            setLocalSampler(uniform, id, value);
        },
        m_uniforms.get(Shader::Uniform::Scope::local)
    };
    callback(localSetter);
    updateLocalDescriptorSet(commandBuffer, id, imageIndex, pipeline);
}

}  // namespace sl

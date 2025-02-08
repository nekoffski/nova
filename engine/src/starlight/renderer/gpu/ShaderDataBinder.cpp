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
      [&](const auto& u, const void* v) { setGlobalUniform(u, v); },
      [&](const auto& u, const Texture* v) { setGlobalSampler(u, v); },
      m_uniforms.get(Shader::Uniform::Scope::global)
    ),
    m_localSetter(
      [&](const auto& u, const void* v) { setLocalUniform(u, v); },
      [&](const auto& u, const Texture* v) { setLocalSampler(u, v); },
      m_uniforms.get(Shader::Uniform::Scope::local)
    ) {}

void ShaderDataBinder::setGlobalUniforms(
  Pipeline& pipeline, CommandBuffer& commandBuffer, u32 imageIndex,
  UniformCallback&& callback
) {
    bindGlobalDescriptor();
    callback(m_globalSetter);
    updateGlobalDescriptor(commandBuffer, imageIndex, pipeline);
}

void ShaderDataBinder::setLocalUniforms(
  Pipeline& pipeline, CommandBuffer& commandBuffer, u32 id, u32 imageIndex,
  UniformCallback&& callback
) {
    bindLocalDescriptor(id);
    callback(m_localSetter);
    updateLocalDescriptor(commandBuffer, imageIndex, pipeline);
}

}  // namespace sl

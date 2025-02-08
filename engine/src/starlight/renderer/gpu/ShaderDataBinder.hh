#pragma once

#include <vector>
#include <functional>

#include "starlight/core/Core.hh"
#include "starlight/core/math/Core.hh"

#include "Texture.hh"
#include "Pipeline.hh"
#include "Shader.hh"

namespace sl {

template <typename T>
concept GlmCompatible = requires(T object) {
    { glm::value_ptr(object) } -> std::convertible_to<void*>;
};

namespace detail {

template <typename T>
requires GlmCompatible<T>
const void* addressOf(const T& value) {
    return glm::value_ptr(value);
}

template <typename T>
requires(not std::is_pointer_v<T> && not GlmCompatible<T>)
const void* addressOf(const T& value) {
    return std::addressof(value);
}

template <typename T> const void* addressOf(const std::vector<T>& vector) {
    return vector.data();
}

template <typename T> const void* addressOf(const T* value) { return value; }

}  // namespace detail

class ShaderDataBinder {
public:
    class Setter {
        using UniformSetter =
          std::function<void(const Shader::Uniform&, const void*)>;
        using SamplerSetter =
          std::function<void(const Shader::Uniform&, const Texture*)>;

    public:
        explicit Setter(
          UniformSetter&& uniformSetter, SamplerSetter&& samplerSetter,
          const Shader::Uniforms::UniformsMap& uniforms
        );

        template <typename T>
        requires(not std::is_same_v<
                 std::remove_pointer_t<std::remove_reference_t<T>>, Texture>)
        void set(const std::string& uniform, T&& value) {
            m_uniformSetter(getUniform(uniform), detail::addressOf(value));
        }

        void set(const std::string& uniform, const Texture* value);

    private:
        const Shader::Uniform& getUniform(const std::string& uniform) const;

        UniformSetter m_uniformSetter;
        SamplerSetter m_samplerSetter;
        const Shader::Uniforms::UniformsMap& m_uniforms;
    };

    explicit ShaderDataBinder(Shader& shader);
    virtual ~ShaderDataBinder() = default;

    using UniformCallback = std::function<void(Setter&)>;

    void setGlobalUniforms(
      Pipeline& pipeline, CommandBuffer& commandBuffer, u32 imageIndex,
      UniformCallback&& callback
    );

    void setLocalUniforms(
      Pipeline& pipeline, CommandBuffer& commandBuffer, u32 id, u32 imageIndex,
      UniformCallback&& callback
    );

    template <typename T>
    void setPushConstant(
      Pipeline& pipeline, CommandBuffer& commandBuffer, const std::string& name,
      T&& value
    ) {
        const auto& uniforms = m_uniforms.get(Shader::Uniform::Scope::pushConstant);
        log::expect(
          uniforms.contains(name), "Could not find '{}' push constant", name
        );
        setPushConstant(
          *uniforms.at(name), detail::addressOf(value), commandBuffer, pipeline
        );
    }

    virtual u32 acquireLocalDescriptor()        = 0;
    virtual void releaseLocalDescriptor(u32 id) = 0;

private:
    virtual void bindGlobalDescriptor()      = 0;
    virtual void bindLocalDescriptor(u32 id) = 0;

    virtual void updateGlobalDescriptor(
      CommandBuffer& commandBuffer, u32 imageIndex, Pipeline& pipeline
    ) = 0;
    virtual void updateLocalDescriptor(
      CommandBuffer& commandBuffer, u32 imageIndex, Pipeline& pipeline
    ) = 0;

    virtual void setLocalSampler(
      const Shader::Uniform& uniform, const Texture* value
    ) = 0;

    virtual void setGlobalSampler(
      const Shader::Uniform& uniform, const Texture* value
    ) = 0;

    virtual void setLocalUniform(
      const Shader::Uniform& uniform, const void* value
    ) = 0;

    virtual void setGlobalUniform(
      const Shader::Uniform& uniform, const void* value
    ) = 0;

    virtual void setPushConstant(
      const Shader::Uniform& uniform, const void* value,
      CommandBuffer& commandBuffer, Pipeline& pipeline
    ) = 0;

    const Shader::Uniforms& m_uniforms;

    Setter m_globalSetter;
    Setter m_localSetter;
};

}  // namespace sl

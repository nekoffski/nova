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
          std::function<bool(const Shader::Uniform&, const void*)>;
        using SamplerSetter =
          std::function<bool(const Shader::Uniform&, const Texture*)>;

    public:
        explicit Setter(
          UniformSetter&& uniformSetter, SamplerSetter&& samplerSetter,
          const Shader::DataLayout::DescriptorSet& descriptorLayout
        );

        template <typename T>
        requires(not std::is_same_v<
                 std::remove_pointer_t<std::remove_reference_t<T>>, Texture>)
        void set(const std::string& uniform, T&& value) {
            static constexpr bool isSampler = false;
            m_updated |= m_uniformSetter(
              getUniform(uniform, isSampler), detail::addressOf(value)
            );
        }

        void set(const std::string& uniform, const Texture* value);
        bool wasUpdated() const;

    private:
        const Shader::Uniform& getUniform(const std::string& uniform, bool isSampler)
          const;

        bool m_updated;

        UniformSetter m_uniformSetter;
        SamplerSetter m_samplerSetter;
        const Shader::DataLayout::DescriptorSet& m_descriptorLayout;
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
        const auto& uniforms = m_dataLayout.pushConstants.nonSamplers;
        log::expect(
          uniforms.contains(name), "Could not find '{}' push constant", name
        );
        setPushConstant(
          uniforms.at(name), detail::addressOf(value), commandBuffer, pipeline
        );
    }

    virtual u32 acquireLocalDescriptorSet()        = 0;
    virtual void releaseLocalDescriptorSet(u32 id) = 0;

protected:
    virtual void bindGlobalDescriptorSet(
      CommandBuffer& commandBuffer, u32 imageIndex, Pipeline& pipeline, bool update
    ) = 0;
    virtual void bindLocalDescriptorSet(
      CommandBuffer& commandBuffer, u32 id, u32 imageIndex, Pipeline& pipeline,
      bool update
    ) = 0;

    virtual bool setLocalSampler(
      const Shader::Uniform& uniform, u32 id, const Texture* value
    ) = 0;

    virtual bool setGlobalSampler(
      const Shader::Uniform& uniform, const Texture* value
    ) = 0;

    virtual bool setLocalUniform(
      const Shader::Uniform& uniform, u32 id, const void* value
    ) = 0;

    virtual bool setGlobalUniform(
      const Shader::Uniform& uniform, const void* value
    ) = 0;

    virtual void setPushConstant(
      const Shader::Uniform& uniform, const void* value,
      CommandBuffer& commandBuffer, Pipeline& pipeline
    ) = 0;

    const Shader::DataLayout& m_dataLayout;
};

}  // namespace sl

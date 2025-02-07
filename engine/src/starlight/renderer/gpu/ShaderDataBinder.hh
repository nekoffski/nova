#pragma once

#include <vector>
#include <functional>

#include "starlight/core/Core.hh"
#include "starlight/core/math/Core.hh"

#include "Texture.hh"

namespace sl {

template <typename T>
concept GlmCompatible = requires(T object) {
    { glm::value_ptr(object) } -> std::convertible_to<void*>;
};

class ShaderDataBinder {
public:
    class Setter {
    public:
        template <typename T>
        requires GlmCompatible<T>
        void set(const std::string& uniform, const T& value) {
            // m_shader.setUniform(uniform, glm::value_ptr(value), m_commandBuffer);
        }

        template <typename T>
        requires(not std::is_pointer_v<T> && not GlmCompatible<T>)
        void set(const std::string& uniform, const T& value) {
            // m_shader.setUniform(uniform, std::addressof(value), m_commandBuffer);
        }

        template <typename T>
        void set(const std::string& uniform, const std::vector<T>& array) {
            // m_shader.setUniform(uniform, array.data(), m_commandBuffer);
        }

        template <typename T> void set(const std::string& uniform, const T* value) {
            // m_shader.setUniform(uniform, value, m_commandBuffer);
        }

        void set(const std::string& uniform, const Texture* value) {}

    private:
    };

    using UniformCallback = std::function<void(Setter&)>;

    void setGlobalUniforms(
      CommandBuffer& commandBuffer, u32 imageIndex, UniformCallback&& callback
    ) {}

    void setLocalUniforms(
      CommandBuffer& commandBuffer, u64 id, u32 imageIndex,
      UniformCallback&& callback
    ) {}

    void setPushContants(CommandBuffer& commandBuffer, UniformCallback&& callback) {}

private:
};

}  // namespace sl

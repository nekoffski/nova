#pragma once

#include <string>
#include <array>

#include "starlight/core/containers/FlatMap.hh"
#include "starlight/core/Core.hh"
#include "starlight/core/Resource.hh"
#include "starlight/core/Utils.hh"

namespace sl {

class Shader : public NonMovable, public Identificable<Shader> {
    static constexpr u32 uniformScopes = 3u;

public:
    static constexpr u32 maxStages         = 3u;
    static constexpr u32 maxGlobalTextures = 32u;
    static constexpr u32 maxLocalTextures  = 32u;
    static constexpr u32 maxAttributes     = 16u;

    static constexpr u32 descriptorSetCount = 2;
    static constexpr u32 uboGlobalSet       = 0u;
    static constexpr u32 uboLocalSet        = 1u;

    enum class DataType : u8 {
        vec2,
        vec3,
        vec4,
        f32,
        i8,
        u8,
        i16,
        u16,
        i32,
        u32,
        mat4,
        sampler,
        boolean,
        custom
    };

    struct InputAttribute {
        u32 location;
        u32 offset;
        DataType type;
        u64 size;
        std::string name;
    };

    struct Uniform {
        enum class Scope : u8 { global = 0, local, pushConstant };

        u32 offset;
        u32 binding;
        DataType type;
        u64 size;
        Scope scope;
        std::string name;
    };

    class Uniforms {
        friend class Shader;

    public:
        using UniformsMap = std::unordered_map<std::string, const Uniform*>;

        const Uniform* get(Uniform::Scope scope, const std::string& name) const;
        const UniformsMap& get(Uniform::Scope scope) const;

    private:
        void add(const Uniform*);

        std::array<UniformsMap, uniformScopes> m_uniformLut;
    };

    struct Stage {
        enum class Type : u8 { vertex, fragment, compute, geometry };

        std::string fullPath;
        std::string sourceCode;
        Type type;
    };

    struct Properties {
        std::vector<Stage> stages;
        std::vector<InputAttribute> inputAttributes;
        std::vector<Uniform> uniforms;
    };

    explicit Shader(const Properties& properties);
    virtual ~Shader() = default;

    const Properties properties;

    u64 getInputAttributesStride() const;
    u64 getPushContantsSize() const;
    u64 getLocalUboSize() const;
    u64 getGlobalUboSize() const;

    const Uniforms& getUniforms() const;

protected:
    Uniforms m_uniforms;

    void processInputAttributes();
    void processUniforms();

    u64 m_inputAttributesStride;
    u64 m_pushConstantsSize;

    u32 m_localSamplerCount;
    u32 m_globalSamplerCount;
    u64 m_localUboSize;
    u64 m_globalUboSize;
};

template <> Shader::DataType fromString<Shader::DataType>(std::string_view str);

std::string toString(Shader::DataType);
std::string toString(Shader::Stage::Type);
std::string toString(Shader::InputAttribute);
std::string toString(Shader::Stage);
std::string toString(Shader::Uniform);
std::string toString(Shader::Uniform::Scope);

}  // namespace sl

// #pragma once

// // #include <string>
// // #include <vector>
// // #include <unordered_map>
// // #include <array>
// // #include <memory>
// // #include <functional>
// // #include <concepts>
// // #include <span>
// // #include <string_view>

// #include "starlight/core/Core.hh"
// #include "starlight/core/Utils.hh"
// #include "starlight/core/memory/Memory.hh"
// #include "starlight/core/Id.hh"
// #include "starlight/core/math/Core.hh"
// #include "starlight/core/Log.hh"

// #include "CommandBuffer.hh"
// // #include "starlight/renderer/fwd.hh"
// // #include "Texture.hh"
// #include "Shader.hh"

// namespace sl {

// class Shader : public NonMovable, public Identificable<Shader> {
// public:
//     class LocalUniformOffset {
//     public:
//     private:
//     };

//     class UniformProxy {
//     public:
//         template <typename T>
//         requires GlmCompatible<T>
//         void set(const std::string& uniform, const T& value) {
//             // m_shader.setUniform(uniform, glm::value_ptr(value),
//             m_commandBuffer);
//         }

//         template <typename T>
//         requires(not std::is_pointer_v<T> && not GlmCompatible<T>)
//         void set(const std::string& uniform, const T& value) {
//             // m_shader.setUniform(uniform, std::addressof(value),
//             m_commandBuffer);
//         }

//         template <typename T>
//         void set(const std::string& uniform, const std::vector<T>& array) {
//             // m_shader.setUniform(uniform, array.data(), m_commandBuffer);
//         }

//         template <typename T> void set(const std::string& uniform, const T* value)
//         {
//             // m_shader.setUniform(uniform, value, m_commandBuffer);
//         }

//         void set(const std::string& uniform, const Texture* value);
//     };

//     using UniformCallback = std::function<void(UniformProxy&)>;

//     void setGlobalUniforms(
//       CommandBuffer& commandBuffer, u32 imageIndex, UniformCallback&& callback
//     );

//     void setLocalUniforms(
//       CommandBuffer& commandBuffer, LocalUniformOffset& localOffset, u32
//       imageIndex, UniformCallback&& callback
//     );

//     void setPushContants(CommandBuffer& commandBuffer, UniformCallback&&
//     callback);

//     void use(CommandBuffer& commandBuffer);

// private:
// };

// }  // namespace sl

// // class Shader : public NonMovable, public Identificable<Shader> {
// // public:
// //     enum class Scope : u8 {
// //         global   = 0,  // updated once per frame
// //         instance = 1,  // per instance
// //         local    = 2   // per object
// //     };

// //     static Scope scopeFromString(const std::string& name);
// //     static std::string scopeToString(Scope scope);

// //     struct Attribute {
// //         enum class Type : u8 {
// //             float32,
// //             float32_2,
// //             float32_3,
// //             float32_4,
// //             mat4,
// //             int8,
// //             uint8,
// //             int16,
// //             uint16,
// //             int32,
// //             uint32
// //         };

// //         static Type typeFromString(const std::string& name);
// //         static std::string typeToString(Type type);
// //         static u32 getTypeSize(Type type);

// //         std::string name;
// //         Type type;
// //         u32 size;
// //     };

// //     struct Stage {
// //         enum class Type : u8 { vertex, geometry, fragment, compute };

// //         static Type typeFromString(const std::string& name);
// //         static std::string typeToString(Type stage);

// //         Type type;
// //         std::string source;
// //     };

// //     struct Uniform {
// //         enum class Type : u8 {
// //             float32,
// //             float32_2,
// //             float32_3,
// //             float32_4,
// //             int8,
// //             uint8,
// //             int16,
// //             uint16,
// //             int32,
// //             uint32,
// //             mat4,
// //             sampler,
// //             custom
// //         };

// //         struct Properties {
// //             std::string name;
// //             u8 size;
// //             u32 location;
// //             Uniform::Type type;
// //             Scope scope;
// //         };

// //         bool isSampler() const;

// //         static Type typeFromString(const std::string& name);
// //         static std::string typeToString(Type type);
// //         static u32 getTypeSize(Type type);

// //         u64 offset;
// //         u16 location;
// //         u16 index;
// //         u16 size;
// //         Id8 setIndex;

// //         Scope scope;
// //         Type type;
// //     };

// //     struct Properties {
// //         bool useInstances;
// //         bool useLocals;

// //         std::vector<Attribute> attributes;
// //         std::vector<Stage> stages;
// //         std::vector<Uniform::Properties> uniformProperties;
// //         Texture* defaultTexture;

// //         CullMode cullMode;
// //         PolygonMode polygonMode;
// //     };

// //     class UniformProxy {
// //         friend class Shader;

// //     public:
// //         template <typename T>
// //         requires GlmCompatible<T>
// //         void set(const std::string& uniform, const T& value) {
// //             m_shader.setUniform(uniform, glm::value_ptr(value),
// m_commandBuffer);
// //         }

// //         template <typename T>
// //         requires(not std::is_pointer_v<T> && not GlmCompatible<T>)
// //         void set(const std::string& uniform, const T& value) {
// //             m_shader.setUniform(uniform, std::addressof(value),
// m_commandBuffer);
// //         }

// //         template <typename T>
// //         void set(const std::string& uniform, const std::vector<T>& array) {
// //             m_shader.setUniform(uniform, array.data(), m_commandBuffer);
// //         }

// //         template <typename T> void set(const std::string& uniform, const T*
// value)
// //         {
// //             m_shader.setUniform(uniform, value, m_commandBuffer);
// //         }

// //         void set(const std::string& uniform, const Texture* value);

// //     private:
// //         explicit UniformProxy(Shader& shader, CommandBuffer& commandBuffer);
// //         Shader& m_shader;
// //         CommandBuffer& m_commandBuffer;
// //     };

// //     class Instance : NonMovable, NonCopyable {
// //         friend class Shader;

// //         struct Record {
// //             u64 instanceId;
// //             ResourceRef<Shader> shader;
// //         };

// //     public:
// //         explicit Instance(const std::vector<Texture*>& textures);
// //         ~Instance();

// //         u32 getId(ResourceRef<Shader> shader);

// //     private:
// //         std::vector<Texture*> m_textures;
// //         std::unordered_map<u64, Record> m_records;
// //     };

// //     using UniformCallback = std::function<void(UniformProxy&)>;

// //     virtual ~Shader() = default;

// //     // FIXME: temporary till shader/pipeline refactor
// //     virtual void bindPipeline(Pipeline& pipeline) = 0;

// //     virtual void use(CommandBuffer&) = 0;

// //     void setGlobalUniforms(
// //       CommandBuffer& commandBuffer, u32 imageIndex, UniformCallback&& callback
// //     );
// //     void setInstanceUniforms(
// //       CommandBuffer& commandBuffer, u32 instanceId, u32 imageIndex,
// //       UniformCallback&& callback
// //     );
// //     void setLocalUniforms(CommandBuffer& commandBuffer, UniformCallback&&
// //     callback);

// // protected:
// //     explicit Shader(const Properties& props);

// // private:
// //     virtual u32 acquireInstanceResources(const std::vector<Texture*>& textures)
// =
// //     0; virtual void releaseInstanceResources(u32 instanceId) = 0;

// //     virtual void bindGlobals()                                               =
// 0;
// //     virtual void bindInstance(u32 instanceId)                                =
// 0;
// //     virtual void applyGlobals(CommandBuffer& commandBuffer, u32 imageIndex)  =
// 0;
// //     virtual void applyInstance(CommandBuffer& commandBuffer, u32 imageIndex) =
// 0;

// //     virtual void setSampler(const std::string& uniform, const Texture* value) =
// 0;
// //     virtual void setUniform(
// //       const std::string& name, const void* value, CommandBuffer& commandBuffer
// //     ) = 0;

// // protected:
// //     std::string m_name;
// //     bool m_useInstances;
// //     bool m_useLocals;

// //     CullMode m_cullMode;
// //     PolygonMode m_polygonMode;
// // };

// // }  // namespace sl

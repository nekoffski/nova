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
    u32 getLocalSamplerCount() const;
    u32 getGlobalSamplerCount() const;
    u32 getLocalUniformCount() const;
    u32 getGlobalUniformCount() const;

    const Uniforms& getUniforms() const;

protected:
    Uniforms m_uniforms;

    void processInputAttributes();
    void processUniforms();

    u64 m_inputAttributesStride;
    u64 m_pushConstantsSize;

    u32 m_globalSamplerCount;
    u32 m_localSamplerCount;
    u32 m_globalUniformCount;
    u32 m_localUniformCount;

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

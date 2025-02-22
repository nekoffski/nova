#pragma once

#include <string>
#include <span>
#include <array>

#include "starlight/core/containers/KeyVector.hh"
#include "starlight/core/Core.hh"
#include "starlight/core/Resource.hh"
#include "starlight/core/Utils.hh"

namespace sl {

namespace detail {

template <typename T> struct NameGetter {
    const std::string& operator()(const T& t) const { return t.name; }
};

}  // namespace detail

class Shader : public NonMovable, public Identificable<Shader> {
    static constexpr u32 uniformScopes = 3u;

public:
    static constexpr u32 maxStages         = 3u;
    static constexpr u32 maxGlobalTextures = 32u;
    static constexpr u32 maxLocalTextures  = 32u;
    static constexpr u32 maxAttributes     = 16u;

    static constexpr u32 descriptorSetCount = 2u;
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

    using UniformMap = KeyVector<Uniform, detail::NameGetter<Uniform>>;

    struct DataLayout {
        explicit DataLayout(
          std::span<const InputAttribute> attributes,
          std::span<const Uniform> uniforms
        );

        struct InputAttributes {
            std::vector<InputAttribute> fields;
            u64 stride = 0u;
        };

        struct PushConstants {
            UniformMap nonSamplers;
            u64 size = 0u;
        };

        struct DescriptorSet {
            UniformMap nonSamplers;
            UniformMap samplers;
            u64 size = 0u;
        };

        InputAttributes inputAttributes;
        PushConstants pushConstants;
        DescriptorSet globalDescriptorSet;
        DescriptorSet localDescriptorSet;
    };

    struct Stage {
        enum class Type : u8 { vertex, fragment, compute, geometry };

        std::string fullPath;
        std::string sourceCode;
        Type type;
    };

    struct Properties {
        std::vector<Stage> stages;
        DataLayout layout;
    };

    static SharedPointer<Shader> create(const Properties& props);

    virtual ~Shader();

    const Properties properties;

protected:
    explicit Shader(const Properties& properties);
};

template <> Shader::DataType fromString<Shader::DataType>(std::string_view str);

std::string toString(const Shader::DataType&);
std::string toString(const Shader::Stage::Type&);
std::string toString(const Shader::InputAttribute&);
std::string toString(const Shader::Stage&);
std::string toString(const Shader::Uniform&);
std::string toString(const Shader::Uniform::Scope&);

void logObject(const Shader::Properties& properties);

}  // namespace sl

#pragma once

#include <string>

#include "starlight/core/Core.hh"
#include "starlight/core/Resource.hh"
#include "starlight/core/Utils.hh"

namespace sl {

class ShaderProgram : public NonMovable, public Identificable<ShaderProgram> {
public:
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

    struct Attribute {
        u32 location;
        DataType type;
        u64 size;
        std::string name;
    };

    struct Uniform {
        enum class Scope : u8 { global = 0, local, pushConstant };

        u32 offset;
        DataType type;
        u64 size;
        Scope scope;
        std::string name;
    };

    struct Stage {
        enum class Type : u8 { vertex, fragment, compute, geometry };

        std::string fullPath;
        std::string sourceCode;
        Type type;
    };

    struct Properties {
        std::vector<Stage> stages;
        std::vector<Attribute> attributes;
        std::vector<Uniform> uniforms;
    };

private:
};

template <>
ShaderProgram::DataType fromString<ShaderProgram::DataType>(std::string_view str);

std::string toString(ShaderProgram::DataType);
std::string toString(ShaderProgram::Stage::Type);
std::string toString(ShaderProgram::Attribute);
std::string toString(ShaderProgram::Stage);
std::string toString(ShaderProgram::Uniform);
std::string toString(ShaderProgram::Uniform::Scope);

}  // namespace sl

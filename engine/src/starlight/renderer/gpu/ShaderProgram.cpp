#include "ShaderProgram.hh"

#include <unordered_map>

namespace sl {

std::string toString(ShaderProgram::DataType type) {
    switch (type) {
        case ShaderProgram::DataType::vec2:
            return "vec2";
        case ShaderProgram::DataType::vec3:
            return "vec3";
        case ShaderProgram::DataType::vec4:
            return "vec4";
        case ShaderProgram::DataType::f32:
            return "f32";
        case ShaderProgram::DataType::i8:
            return "i8";
        case ShaderProgram::DataType::u8:
            return "u8";
        case ShaderProgram::DataType::i16:
            return "i16";
        case ShaderProgram::DataType::u16:
            return "u16";
        case ShaderProgram::DataType::i32:
            return "i32";
        case ShaderProgram::DataType::u32:
            return "u32";
        case ShaderProgram::DataType::mat4:
            return "mat4";
        case ShaderProgram::DataType::sampler:
            return "sampler";
        case ShaderProgram::DataType::custom:
            return "custom";
        case ShaderProgram::DataType::boolean:
            return "boolean";
    }
    log::panic("Invalid data type: {}", fmt::underlying(type));
}

std::string toString(ShaderProgram::Stage::Type type) {
    switch (type) {
        case ShaderProgram::Stage::Type::vertex:
            return "VERTEX";
        case ShaderProgram::Stage::Type::geometry:
            return "GEOMETRY";
        case ShaderProgram::Stage::Type::fragment:
            return "FRAGMENT";
        case ShaderProgram::Stage::Type::compute:
            return "COMPUTE";
    }
    log::panic("Invalid stage type: {}", fmt::underlying(type));
}

std::string toString(ShaderProgram::Attribute attribute) {
    return fmt::format(
      "Attribute: location {:02}. / {:02}b / {:5} / '{}'", attribute.location,
      attribute.size, attribute.type, attribute.name
    );
}

std::string toString(ShaderProgram::Stage stage) {
    return fmt::format("Stage: {} - {}", stage.type, stage.fullPath);
}

std::string toString(ShaderProgram::Uniform uniform) {
    return fmt::format(
      "Uniform: {:>13} / offset {:04}. / {:04}b / {:7} / '{}'", uniform.scope,
      uniform.offset, uniform.size, uniform.type, uniform.name
    );
}

std::string toString(ShaderProgram::Uniform::Scope scope) {
    switch (scope) {
        case ShaderProgram::Uniform::Scope::global:
            return "GLOBAL";
        case ShaderProgram::Uniform::Scope::local:
            return "LOCAL";
        case ShaderProgram::Uniform::Scope::pushConstant:
            return "PUSH_CONSTANT";
    }
    log::panic("Invalid uniform scope type: {}", fmt::underlying(scope));
}

template <>
ShaderProgram::DataType fromString<ShaderProgram::DataType>(std::string_view str) {
    static std::unordered_map<std::string_view, ShaderProgram::DataType> lut{
        { "vec2",    ShaderProgram::DataType::vec2    },
        { "vec3",    ShaderProgram::DataType::vec3    },
        { "vec4",    ShaderProgram::DataType::vec4    },
        { "f32",     ShaderProgram::DataType::f32     },
        { "i8",      ShaderProgram::DataType::i8      },
        { "u8",      ShaderProgram::DataType::u8      },
        { "i16",     ShaderProgram::DataType::i16     },
        { "u16",     ShaderProgram::DataType::u16     },
        { "i32",     ShaderProgram::DataType::i32     },
        { "u32",     ShaderProgram::DataType::u32     },
        { "mat4",    ShaderProgram::DataType::mat4    },
        { "sampler", ShaderProgram::DataType::sampler },
        { "custom",  ShaderProgram::DataType::custom  }
    };
    if (auto it = lut.find(str); it != lut.end()) [[likely]]
        return it->second;
    log::panic("Could not parse ShaderProgram::DataType from '{}'", str);
}

}  // namespace sl

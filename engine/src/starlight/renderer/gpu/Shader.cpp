#include "Shader.hh"

#include <unordered_map>

namespace sl {

Shader::Shader(const Properties& properties
) : properties(properties), m_inputAttributesStride(0u), m_pushConstantsSize(0u) {
    processInputAttributes();
    processUniforms();
}

void Shader::processInputAttributes() {
    for (auto& attribute : properties.inputAttributes)
        m_inputAttributesStride += attribute.size;
}

void Shader::processUniforms() {
    for (const auto& uniform : properties.uniforms) {
        m_uniformLut.at(static_cast<u8>(uniform.scope))[uniform.name] = &uniform;

        if (uniform.scope == Uniform::Scope::pushConstant)
            m_pushConstantsSize += uniform.size;
    }
}

u64 Shader::getPushContantsSize() const { return m_pushConstantsSize; }

const Shader::Uniforms& Shader::getUniforms(Uniform::Scope scope) const {
    const auto index = fmt::underlying(scope);
    log::expect(index < uniformScopes, "Invalid uniform scope: {}", index);
    return m_uniformLut.at(index);
}

u64 Shader::getInputAttributesStride() const { return m_inputAttributesStride; }

std::string toString(Shader::DataType type) {
    switch (type) {
        case Shader::DataType::vec2:
            return "vec2";
        case Shader::DataType::vec3:
            return "vec3";
        case Shader::DataType::vec4:
            return "vec4";
        case Shader::DataType::f32:
            return "f32";
        case Shader::DataType::i8:
            return "i8";
        case Shader::DataType::u8:
            return "u8";
        case Shader::DataType::i16:
            return "i16";
        case Shader::DataType::u16:
            return "u16";
        case Shader::DataType::i32:
            return "i32";
        case Shader::DataType::u32:
            return "u32";
        case Shader::DataType::mat4:
            return "mat4";
        case Shader::DataType::sampler:
            return "sampler";
        case Shader::DataType::custom:
            return "custom";
        case Shader::DataType::boolean:
            return "boolean";
    }
    log::panic("Invalid data type: {}", fmt::underlying(type));
}

std::string toString(Shader::Stage::Type type) {
    switch (type) {
        case Shader::Stage::Type::vertex:
            return "VERTEX";
        case Shader::Stage::Type::geometry:
            return "GEOMETRY";
        case Shader::Stage::Type::fragment:
            return "FRAGMENT";
        case Shader::Stage::Type::compute:
            return "COMPUTE";
    }
    log::panic("Invalid stage type: {}", fmt::underlying(type));
}

std::string toString(Shader::InputAttribute attribute) {
    return fmt::format(
      "InputAttribute: location {:02}. / offset {:04}. / {:02}b / {:5} / '{}'",
      attribute.location, attribute.offset, attribute.size, attribute.type,
      attribute.name
    );
}

std::string toString(Shader::Stage stage) {
    return fmt::format("Stage: {} - {}", stage.type, stage.fullPath);
}

std::string toString(Shader::Uniform uniform) {
    return fmt::format(
      "Uniform: {:>13} / offset {:04}. / {:04}b / {:7} / '{}'", uniform.scope,
      uniform.offset, uniform.size, uniform.type, uniform.name
    );
}

std::string toString(Shader::Uniform::Scope scope) {
    switch (scope) {
        case Shader::Uniform::Scope::global:
            return "GLOBAL";
        case Shader::Uniform::Scope::local:
            return "LOCAL";
        case Shader::Uniform::Scope::pushConstant:
            return "PUSH_CONSTANT";
    }
    log::panic("Invalid uniform scope type: {}", fmt::underlying(scope));
}

template <> Shader::DataType fromString<Shader::DataType>(std::string_view str) {
    static std::unordered_map<std::string_view, Shader::DataType> lut{
        { "vec2",    Shader::DataType::vec2    },
        { "vec3",    Shader::DataType::vec3    },
        { "vec4",    Shader::DataType::vec4    },
        { "f32",     Shader::DataType::f32     },
        { "i8",      Shader::DataType::i8      },
        { "u8",      Shader::DataType::u8      },
        { "i16",     Shader::DataType::i16     },
        { "u16",     Shader::DataType::u16     },
        { "i32",     Shader::DataType::i32     },
        { "u32",     Shader::DataType::u32     },
        { "mat4",    Shader::DataType::mat4    },
        { "sampler", Shader::DataType::sampler },
        { "custom",  Shader::DataType::custom  }
    };
    if (auto it = lut.find(str); it != lut.end()) [[likely]]
        return it->second;
    log::panic("Could not parse Shader::DataType from '{}'", str);
}

}  // namespace sl

// #include "Shader.hh"

// #include "starlight/core/Log.hh"

// #include <numeric>
// #include <ranges>
// #include <concepts>

// #include <fmt/core.h>

// #include "starlight/core/Json.hh"
// #include "starlight/renderer/factories/TextureFactory.hh"

// namespace sl {

// Shader::Scope Shader::scopeFromString(const std::string& name) {
//     if (name == "local")
//         return Shader::Scope::local;
//     else if (name == "instance")
//         return Shader::Scope::instance;
//     else if (name == "global")
//         return Shader::Scope::global;
//     log::panic("Could not parse shader scope: {}", name);
// }

// std::string Shader::scopeToString(Shader::Scope scope) {
//     switch (scope) {
//         case Shader::Scope::global:
//             return "global";
//         case Shader::Scope::instance:
//             return "instance";
//         case Shader::Scope::local:
//             return "local";
//     }
//     __builtin_unreachable();
// }

// Shader::Attribute::Type Shader::Attribute::typeFromString(const std::string& name)
// {
//     static const std::unordered_map<std::string_view, Shader::Attribute::Type>
//       nameToType{
//           { "f32",  Shader::Attribute::Type::float32   },
//           { "vec2", Shader::Attribute::Type::float32_2 },
//           { "vec3", Shader::Attribute::Type::float32_3 },
//           { "vec4", Shader::Attribute::Type::float32_4 },
//           { "u8",   Shader::Attribute::Type::uint8     },
//           { "u16",  Shader::Attribute::Type::uint16    },
//           { "u32",  Shader::Attribute::Type::uint32    },
//           { "i8",   Shader::Attribute::Type::int8      },
//           { "i16",  Shader::Attribute::Type::int16     },
//           { "i32",  Shader::Attribute::Type::int32     },
//     };
//     const auto record = nameToType.find(name);
//     log::expect(record != nameToType.end(), "Invalid type attribute name: {}",
//     name); return record->second;
// }

// std::string Shader::Attribute::typeToString(Type type) {
//     static const std::unordered_map<Shader::Attribute::Type, std::string>
//     typeToName{
//         { Shader::Attribute::Type::float32,   "f32"  },
//         { Shader::Attribute::Type::float32_2, "vec2" },
//         { Shader::Attribute::Type::float32_3, "vec3" },
//         { Shader::Attribute::Type::float32_4, "vec4" },
//         { Shader::Attribute::Type::uint8,     "u8"   },
//         { Shader::Attribute::Type::uint16,    "u16"  },
//         { Shader::Attribute::Type::uint32,    "u32"  },
//         { Shader::Attribute::Type::int8,      "i8"   },
//         { Shader::Attribute::Type::int16,     "i16"  },
//         { Shader::Attribute::Type::int32,     "i32"  },
//     };
//     const auto record = typeToName.find(type);
//     log::expect(
//       record != typeToName.end(), "Invalid attribute type: {}",
//       fmt::underlying(type)
//     );
//     return record->second;
// }

// u32 Shader::Attribute::getTypeSize(Type type) {
//     static const std::unordered_map<Shader::Attribute::Type, u32> typeToSize{
//         { Shader::Attribute::Type::float32,   4  },
//         { Shader::Attribute::Type::float32_2, 8  },
//         { Shader::Attribute::Type::float32_3, 12 },
//         { Shader::Attribute::Type::float32_4, 16 },
//         { Shader::Attribute::Type::uint8,     1  },
//         { Shader::Attribute::Type::uint16,    2  },
//         { Shader::Attribute::Type::uint32,    4  },
//         { Shader::Attribute::Type::int8,      1  },
//         { Shader::Attribute::Type::int16,     2  },
//         { Shader::Attribute::Type::int32,     4  },
//     };
//     const auto record = typeToSize.find(type);
//     log::expect(
//       record != typeToSize.end(), "Invalid attribute type: {}",
//       fmt::underlying(type)
//     );
//     return record->second;
// }

// Shader::Stage::Type Shader::Stage::typeFromString(const std::string& name) {
//     if (name == "vertex")
//         return Shader::Stage::Type::vertex;
//     else if (name == "geometry")
//         return Shader::Stage::Type::geometry;
//     else if (name == "fragment")
//         return Shader::Stage::Type::fragment;
//     else if (name == "compute")
//         return Shader::Stage::Type::compute;
//     log::panic("Invalid stage name: {}", name);
// }

// std::string Shader::Stage::typeToString(Shader::Stage::Type type) {
//     switch (type) {
//         case Shader::Stage::Type::vertex:
//             return "vertex";
//         case Shader::Stage::Type::geometry:
//             return "geometry";
//         case Shader::Stage::Type::fragment:
//             return "fragment";
//         case Shader::Stage::Type::compute:
//             return "compute";
//     }
//     log::panic("Invalid stage type: {}", fmt::underlying(type));
// }

// bool Shader::Uniform::isSampler() const { return type == Type::sampler; }

// Shader::Uniform::Type Shader::Uniform::typeFromString(const std::string& name) {
//     static const std::unordered_map<std::string_view, Shader::Uniform::Type>
//       nameToType{
//           { "f32",    Shader::Uniform::Type::float32   },
//           { "vec2",   Shader::Uniform::Type::float32_2 },
//           { "vec3",   Shader::Uniform::Type::float32_3 },
//           { "vec4",   Shader::Uniform::Type::float32_4 },
//           { "u8",     Shader::Uniform::Type::uint8     },
//           { "u16",    Shader::Uniform::Type::uint16    },
//           { "u32",    Shader::Uniform::Type::uint32    },
//           { "i8",     Shader::Uniform::Type::int8      },
//           { "i16",    Shader::Uniform::Type::int16     },
//           { "i32",    Shader::Uniform::Type::int32     },
//           { "mat4",   Shader::Uniform::Type::mat4      },
//           { "samp",   Shader::Uniform::Type::sampler   },
//           { "custom", Shader::Uniform::Type::custom    }
//     };
//     const auto record = nameToType.find(name);
//     log::expect(record != nameToType.end(), "Invalid type Uniform name: {}",
//     name); return record->second;
// }

// std::string Shader::Uniform::typeToString(Type type) {
//     static const std::unordered_map<Shader::Uniform::Type, std::string>
//     typeToName{
//         { Shader::Uniform::Type::float32,   "f32"    },
//         { Shader::Uniform::Type::float32_2, "vec2"   },
//         { Shader::Uniform::Type::float32_3, "vec3"   },
//         { Shader::Uniform::Type::float32_4, "vec4"   },
//         { Shader::Uniform::Type::uint8,     "u8"     },
//         { Shader::Uniform::Type::uint16,    "u16"    },
//         { Shader::Uniform::Type::uint32,    "u32"    },
//         { Shader::Uniform::Type::int8,      "i8"     },
//         { Shader::Uniform::Type::int16,     "i16"    },
//         { Shader::Uniform::Type::int32,     "i32"    },
//         { Shader::Uniform::Type::mat4,      "mat4"   },
//         { Shader::Uniform::Type::sampler,   "samp"   },
//         { Shader::Uniform::Type::custom,    "custom" },
//     };
//     const auto record = typeToName.find(type);
//     log::expect(
//       record != typeToName.end(), "Invalid Uniform type: {}",
//       fmt::underlying(type)
//     );
//     return record->second;
// }

// u32 Shader::Uniform::getTypeSize(Type type) {
//     static const std::unordered_map<Shader::Uniform::Type, u32> typeToSize{
//         { Shader::Uniform::Type::float32,   4  },
//         { Shader::Uniform::Type::float32_2, 8  },
//         { Shader::Uniform::Type::float32_3, 12 },
//         { Shader::Uniform::Type::float32_4, 16 },
//         { Shader::Uniform::Type::uint8,     1  },
//         { Shader::Uniform::Type::uint16,    2  },
//         { Shader::Uniform::Type::uint32,    4  },
//         { Shader::Uniform::Type::int8,      1  },
//         { Shader::Uniform::Type::int16,     2  },
//         { Shader::Uniform::Type::int32,     4  },
//         { Shader::Uniform::Type::mat4,      64 },
//         { Shader::Uniform::Type::sampler,   0  },
//         { Shader::Uniform::Type::custom,    0  }
//     };
//     const auto record = typeToSize.find(type);
//     log::expect(
//       record != typeToSize.end(), "Invalid Uniform type: {}",
//       fmt::underlying(type)
//     );
//     return record->second;
// }

// void Shader::setGlobalUniforms(
//   CommandBuffer& commandBuffer, u32 imageIndex, UniformCallback&& callback
// ) {
//     bindGlobals();
//     UniformProxy proxy{ *this, commandBuffer };
//     callback(proxy);
//     applyGlobals(commandBuffer, imageIndex);
// }

// void Shader::setInstanceUniforms(
//   CommandBuffer& commandBuffer, u32 instanceId, u32 imageIndex,
//   UniformCallback&& callback
// ) {
//     bindInstance(instanceId);
//     UniformProxy proxy{ *this, commandBuffer };
//     callback(proxy);
//     applyInstance(commandBuffer, imageIndex);
// }

// void Shader::setLocalUniforms(
//   CommandBuffer& commandBuffer, UniformCallback&& callback
// ) {
//     // nothing to do here, just for consistency
//     UniformProxy proxy{ *this, commandBuffer };
//     callback(proxy);
// }

// Shader::Shader(const Properties& props) :
//     m_useInstances(props.useInstances), m_useLocals(props.useLocals),
//     m_cullMode(props.cullMode), m_polygonMode(props.polygonMode) {}

// void Shader::UniformProxy::set(const std::string& uniform, const Texture* value) {
//     m_shader.setSampler(uniform, value);
// }

// Shader::UniformProxy::UniformProxy(Shader& shader, CommandBuffer& commandBuffer) :
//     m_shader(shader), m_commandBuffer(commandBuffer) {}

// Shader::Instance::Instance(const std::vector<Texture*>& textures
// ) : m_textures(textures) {}

// Shader::Instance::~Instance() {
//     for (auto& [instanceId, shader] : m_records | std::views::values)
//         shader->releaseInstanceResources(instanceId);
// }

// u32 Shader::Instance::getId(ResourceRef<Shader> shader) {
//     const auto shaderId = shader->getId();

//     if (const auto record = m_records.find(shaderId); record != m_records.end())
//       [[likely]] {
//         return record->second.instanceId;
//     }

//     const auto instanceId = shader->acquireInstanceResources(m_textures);
//     m_records[shaderId]   = Record{ instanceId, shader };
//     return instanceId;
// }

// }  // namespace sl

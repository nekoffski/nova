#include "Shader.h"

namespace sl {

Shader::Scope Shader::scopeFromString(const std::string& name) {
    if (name == "local")
        return Shader::Scope::local;
    else if (name == "instance")
        return Shader::Scope::instance;
    else if (name == "global")
        return Shader::Scope::global;
    FAIL("Could not parse shader scope: {}", name);
}

std::string Shader::scopeToString(Shader::Scope scope) {
    switch (scope) {
        case Shader::Scope::global:
            return "global";
        case Shader::Scope::instance:
            return "instance";
        case Shader::Scope::local:
            return "local";
    }
    __builtin_unreachable();
}

Shader::Attribute::Type Shader::Attribute::typeFromString(const std::string& name) {
    static const std::unordered_map<std::string_view, Shader::Attribute::Type>
      nameToType{
          {"f32",   Shader::Attribute::Type::float32  },
          { "vec2", Shader::Attribute::Type::float32_2},
          { "vec3", Shader::Attribute::Type::float32_3},
          { "vec4", Shader::Attribute::Type::float32_4},
          { "u8",   Shader::Attribute::Type::uint8    },
          { "u16",  Shader::Attribute::Type::uint16   },
          { "u32",  Shader::Attribute::Type::uint32   },
          { "i8",   Shader::Attribute::Type::int8     },
          { "i16",  Shader::Attribute::Type::int16    },
          { "i32",  Shader::Attribute::Type::int32    },
    };
    const auto record = nameToType.find(name);
    ASSERT(record != nameToType.end(), "Invalid type attribute name: {}", name);
    return record->second;
}

std::string Shader::Attribute::typeToString(Type type) {
    static const std::unordered_map<Shader::Attribute::Type, std::string> typeToName{
        {Shader::Attribute::Type::float32,    "f32" },
        { Shader::Attribute::Type::float32_2, "vec2"},
        { Shader::Attribute::Type::float32_3, "vec3"},
        { Shader::Attribute::Type::float32_4, "vec4"},
        { Shader::Attribute::Type::uint8,     "u8"  },
        { Shader::Attribute::Type::uint16,    "u16" },
        { Shader::Attribute::Type::uint32,    "u32" },
        { Shader::Attribute::Type::int8,      "i8"  },
        { Shader::Attribute::Type::int16,     "i16" },
        { Shader::Attribute::Type::int32,     "i32" },
    };
    const auto record = typeToName.find(type);
    ASSERT(record != typeToName.end(), "Invalid attribute type: {}", type);
    return record->second;
}

u32 Shader::Attribute::getTypeSize(Type type) {
    static const std::unordered_map<Shader::Attribute::Type, u32> typeToSize{
        {Shader::Attribute::Type::float32,    4 },
        { Shader::Attribute::Type::float32_2, 8 },
        { Shader::Attribute::Type::float32_3, 12},
        { Shader::Attribute::Type::float32_4, 16},
        { Shader::Attribute::Type::uint8,     1 },
        { Shader::Attribute::Type::uint16,    2 },
        { Shader::Attribute::Type::uint32,    4 },
        { Shader::Attribute::Type::int8,      1 },
        { Shader::Attribute::Type::int16,     2 },
        { Shader::Attribute::Type::int32,     4 },
    };
    const auto record = typeToSize.find(type);
    ASSERT(record != typeToSize.end(), "Invalid attribute type: {}", type);
    return record->second;
}

Shader::Stage::Type Shader::Stage::typeFromString(const std::string& name) {
    if (name == "vertex")
        return Shader::Stage::Type::vertex;
    else if (name == "geometry")
        return Shader::Stage::Type::geometry;
    else if (name == "fragment")
        return Shader::Stage::Type::fragment;
    else if (name == "compute")
        return Shader::Stage::Type::compute;
    FAIL("Invalid stage name: {}", name);
}

std::string Shader::Stage::typeToString(Shader::Stage::Type type) {
    switch (type) {
        case Shader::Stage::Type::vertex:
            return "vertex";
        case Shader::Stage::Type::geometry:
            return "geometry";
        case Shader::Stage::Type::fragment:
            return "fragment";
        case Shader::Stage::Type::compute:
            return "compute";
    }
    FAIL("Invalid stage type: {}", type);
}

bool Shader::Uniform::isSampler() const { return type == Type::sampler; }

Shader::Uniform::Type Shader::Uniform::typeFromString(const std::string& name) {
    static const std::unordered_map<std::string_view, Shader::Uniform::Type>
      nameToType{
          {"f32",   Shader::Uniform::Type::float32  },
          { "vec2", Shader::Uniform::Type::float32_2},
          { "vec3", Shader::Uniform::Type::float32_3},
          { "vec4", Shader::Uniform::Type::float32_4},
          { "u8",   Shader::Uniform::Type::uint8    },
          { "u16",  Shader::Uniform::Type::uint16   },
          { "u32",  Shader::Uniform::Type::uint32   },
          { "i8",   Shader::Uniform::Type::int8     },
          { "i16",  Shader::Uniform::Type::int16    },
          { "i32",  Shader::Uniform::Type::int32    },
          { "mat4", Shader::Uniform::Type::mat4     },
          { "samp", Shader::Uniform::Type::sampler  },
    };
    const auto record = nameToType.find(name);
    ASSERT(record != nameToType.end(), "Invalid type Uniform name: {}", name);
    return record->second;
}

std::string Shader::Uniform::typeToString(Type type) {
    static const std::unordered_map<Shader::Uniform::Type, std::string> typeToName{
        {Shader::Uniform::Type::float32,    "f32" },
        { Shader::Uniform::Type::float32_2, "vec2"},
        { Shader::Uniform::Type::float32_3, "vec3"},
        { Shader::Uniform::Type::float32_4, "vec4"},
        { Shader::Uniform::Type::uint8,     "u8"  },
        { Shader::Uniform::Type::uint16,    "u16" },
        { Shader::Uniform::Type::uint32,    "u32" },
        { Shader::Uniform::Type::int8,      "i8"  },
        { Shader::Uniform::Type::int16,     "i16" },
        { Shader::Uniform::Type::int32,     "i32" },
        { Shader::Uniform::Type::mat4,      "mat4"},
        { Shader::Uniform::Type::sampler,   "samp"},
    };
    const auto record = typeToName.find(type);
    ASSERT(record != typeToName.end(), "Invalid Uniform type: {}", type);
    return record->second;
}

u32 Shader::Uniform::getTypeSize(Type type) {
    static const std::unordered_map<Shader::Uniform::Type, u32> typeToSize{
        {Shader::Uniform::Type::float32,    4 },
        { Shader::Uniform::Type::float32_2, 8 },
        { Shader::Uniform::Type::float32_3, 12},
        { Shader::Uniform::Type::float32_4, 16},
        { Shader::Uniform::Type::uint8,     1 },
        { Shader::Uniform::Type::uint16,    2 },
        { Shader::Uniform::Type::uint32,    4 },
        { Shader::Uniform::Type::int8,      1 },
        { Shader::Uniform::Type::int16,     2 },
        { Shader::Uniform::Type::int32,     4 },
        { Shader::Uniform::Type::mat4,      64},
        { Shader::Uniform::Type::sampler,   0 },
    };
    const auto record = typeToSize.find(type);
    ASSERT(record != typeToSize.end(), "Invalid Uniform type: {}", type);
    return record->second;
}

void Shader::setGlobalUniforms(SelfCallback&& callback) {
    bindGlobals();
    callback(this);
    applyGlobals();
}

void Shader::setInstanceUniforms(u32 instanceId, SelfCallback&& callback) {
    bindInstance(instanceId);
    callback(this);
    applyInstance();
}

void Shader::setLocalUniforms(SelfCallback&& callback) {
    // nothing to do here, just for consistency
    callback(this);
}

u32 Shader::getId() const { return m_id; }

const std::string& Shader::getName() const { return m_name; }

Shader::Shader(const Properties& props, u32 id) :
    m_name(props.name), m_id(id), m_useInstances(props.useInstances),
    m_useLocals(props.useLocals) {}

void Shader::setUniform(const std::string& uniform, Texture* value) {
    setUniform(uniform, static_cast<void*>(value));
}

}  // namespace sl

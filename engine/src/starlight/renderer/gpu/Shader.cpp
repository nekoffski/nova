#include "Shader.hh"

#include <unordered_map>

namespace sl {

Shader::Shader(const Properties& properties) :
    properties(properties), m_inputAttributesStride(0u), m_pushConstantsSize(0u),
    m_globalSamplerCount(0u), m_localSamplerCount(0u), m_globalUniformCount(0u),
    m_localUniformCount(0u), m_localUboSize(0u), m_globalUboSize(0u) {
    const auto stageCount = properties.stages.size();
    log::expect(
      stageCount <= maxStages, "Max shader stages ({}) exceed: {}", maxStages,
      stageCount
    );

    processInputAttributes();
    processUniforms();
}

void Shader::processInputAttributes() {
    const auto attributeCount = properties.inputAttributes.size();
    log::expect(
      attributeCount < maxAttributes, "Max attribute count ({}) exceed: {}",
      maxAttributes, attributeCount
    );

    for (auto& attribute : properties.inputAttributes)
        m_inputAttributesStride += attribute.size;
}

const Shader::Uniform* Shader::Uniforms::get(
  Uniform::Scope scope, const std::string& name
) const {
    const auto& map   = get(scope);
    const auto record = map.find(name);
    return record != map.end() ? record->second : nullptr;
}

const Shader::Uniforms::UniformsMap& Shader::Uniforms::get(Uniform::Scope scope
) const {
    log::expect(
      scope <= Uniform::Scope::pushConstant,
      "Could not get uniform, invalid scope: {}", fmt::underlying(scope)
    );
    return m_uniformLut.at(static_cast<u8>(scope));
}

void Shader::Uniforms::add(const Uniform* uniform) {
    log::expect(
      fmt::underlying(uniform->scope) <= fmt::underlying(Uniform::Scope::pushConstant
      ),
      "Could not add uniform, invalid scope: {}", fmt::underlying(uniform->scope)
    );
    m_uniformLut.at(static_cast<u8>(uniform->scope))[uniform->name] = uniform;
}

void Shader::processUniforms() {
    for (const auto& uniform : properties.uniforms) {
        m_uniforms.add(&uniform);

        if (uniform.type == DataType::sampler) {
            if (uniform.scope == Uniform::Scope::global)
                m_globalSamplerCount += uniform.size;
            else if (uniform.scope == Uniform::Scope::local)
                m_localSamplerCount += uniform.size;
            else
                log::panic("PushConstant samplers not supported");
        } else {
            if (uniform.scope == Uniform::Scope::global) {
                m_globalUboSize += uniform.size;
                m_globalUniformCount++;
            } else if (uniform.scope == Uniform::Scope::local) {
                m_localUboSize += uniform.size;
                m_localUniformCount++;
            }
        }

        if (uniform.scope == Uniform::Scope::pushConstant)
            m_pushConstantsSize += uniform.size;
    }

    log::expect(
      m_globalSamplerCount < maxGlobalTextures,
      "Max global sampler count ({}) exceed: {}", maxGlobalTextures,
      m_globalSamplerCount
    );
    log::expect(
      m_localSamplerCount < maxLocalTextures,
      "Max local sampler count ({}) exceed: {}", maxLocalTextures,
      m_localSamplerCount
    );
    log::debug(
      "Shader uniforms stats - global samplers: {}, local samplers: {}, global UBO size: {}b, local UBO size: {}b",
      m_globalSamplerCount, m_localSamplerCount, m_globalUboSize, m_localUboSize
    );
}

u64 Shader::getPushContantsSize() const { return m_pushConstantsSize; }

u64 Shader::getLocalUboSize() const { return m_localUboSize; }

u64 Shader::getGlobalUboSize() const { return m_globalUboSize; }

u32 Shader::getLocalSamplerCount() const { return m_localSamplerCount; }

u32 Shader::getGlobalSamplerCount() const { return m_globalSamplerCount; }

u32 Shader::getLocalUniformCount() const { return m_localUniformCount; }

u32 Shader::getGlobalUniformCount() const { return m_globalUniformCount; }

const Shader::Uniforms& Shader::getUniforms() const { return m_uniforms; }

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
      "Uniform: {:>13} / binding {:02} / offset {:04}. / {:04}b / {:7} / '{}'",
      uniform.scope, uniform.binding, uniform.offset, uniform.size, uniform.type,
      uniform.name
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

#include "Shader.hh"

#include <ranges>
#include <unordered_map>

#ifdef SL_USE_VK
#include "vulkan/VulkanDevice.hh"
#include "vulkan/VulkanShader.hh"
#endif

namespace sl {

SharedPtr<Shader> Shader::create(const Properties& props) {
#ifdef SL_USE_VK
    return SharedPtr<vk::VulkanShader>::create(
      static_cast<vk::VulkanDevice&>(Device::get().getImpl()), props
    );
#else
    log::panic("GPU API vendor not specified");
#endif
}

Shader::~Shader() { log::trace("Destroying shader: {}", id); }

Shader::Shader(const Properties& properties) : properties(properties) {
    const auto stageCount = properties.stages.size();
    log::expect(
      stageCount <= maxStages, "Max shader stages ({}) exceed: {}", maxStages,
      stageCount
    );
}

std::string toString(const Shader::DataType& type) {
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

std::string toString(const Shader::Stage::Type& type) {
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

std::string toString(const Shader::InputAttribute& attribute) {
    return fmt::format(
      "{:02}. / offset {:04}. / {:02}b / {:5} / '{}'", attribute.location,
      attribute.offset, attribute.size, attribute.type, attribute.name
    );
}

std::string toString(const Shader::Stage& stage) {
    return fmt::format("{} - {}", stage.type, stage.fullPath);
}

std::string toString(const Shader::Uniform& uniform) {
    return fmt::format(
      "{} / binding {:02} / offset {:04}. / {:04}b / {:7} / '{}'", uniform.scope,
      uniform.binding, uniform.offset, uniform.size, uniform.type, uniform.name
    );
}

std::string toString(const Shader::Uniform::Scope& scope) {
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

void logObject(const Shader::Properties& properties) {
    log::debug("ShaderProperties {");
    log::debug("{}Stages:", spaces(2));
    for (const auto& stage : properties.stages) log::debug("{}{}", spaces(4), stage);

    const auto& layout = properties.layout;
    log::debug(
      "{}Input Attributes (total stride - {}b):", spaces(2),
      layout.inputAttributes.stride
    );
    for (const auto& attribute : layout.inputAttributes.fields)
        log::debug("{}{}", spaces(4), attribute);

    log::debug(
      "{}Push Constants (total size - {}b):", spaces(2), layout.pushConstants.size
    );
    layout.pushConstants.nonSamplers.forEach([](const auto& field) {
        log::debug("{}{}", spaces(4), field);
    });

    static auto logDescriptorSet = [](const Shader::DataLayout::DescriptorSet& set) {
        log::debug("{}Non-Samplers:", spaces(6));
        set.nonSamplers.forEach([](const auto& field) {
            log::debug("{}{}", spaces(8), field);
        });

        log::debug("{}Samplers:", spaces(6));
        set.samplers.forEach([](const auto& field) {
            log::debug("{}{}", spaces(8), field);
        });
    };

    log::debug("{}Uniforms:", spaces(2));
    log::debug(
      "{} Local Descriptor Set (total size - {}b):", spaces(4),
      layout.localDescriptorSet.size
    );
    logDescriptorSet(layout.localDescriptorSet);

    log::debug(
      "{} Global Descriptor Set (total size - {}b):", spaces(4),
      layout.globalDescriptorSet.size
    );
    logDescriptorSet(layout.globalDescriptorSet);

    log::debug("}");
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

/*
    Shader::DataLayout
*/

static void calculateSamplersOffsets(Shader::UniformMap& samplers) {
    samplers.forEach([index = 0u](Shader::Uniform& sampler) mutable {
        sampler.offset = index++;
    });
}

Shader::DataLayout::DataLayout(
  std::span<const InputAttribute> attributes, std::span<const Uniform> uniforms
) {
    std::ranges::copy(attributes, into(inputAttributes.fields));

    std::ranges::sort(
      inputAttributes.fields,
      [](const auto& lhs, const auto& rhs) -> bool {
          return lhs.location < rhs.location;
      }
    );

    for (auto& attribute : inputAttributes.fields) {
        attribute.offset = inputAttributes.stride;
        inputAttributes.stride += attribute.size;
    }

    std::array<DescriptorSet*, 2> lut{ &localDescriptorSet, &globalDescriptorSet };

    auto getDescriptorSet = [&](const auto scope) {
        return lut[static_cast<u32>(scope == Uniform::Scope::global)];
    };

    for (const auto& uniform : uniforms) {
        if (uniform.scope == Uniform::Scope::pushConstant) {
            pushConstants.nonSamplers.push(uniform);
            pushConstants.size += uniform.size;
        } else {
            auto set = getDescriptorSet(uniform.scope);
            if (uniform.type == DataType::sampler) {
                set->samplers.push(uniform);
            } else {
                set->nonSamplers.push(uniform);
                set->size += uniform.size;
            }
        }
    }
    calculateSamplersOffsets(localDescriptorSet.samplers);
    calculateSamplersOffsets(globalDescriptorSet.samplers);
}

}  // namespace sl

#include "SPIRVParser.hh"

#include <stdexcept>
#include <ranges>

#include <iostream>

#include <fmt/core.h>

#include "starlight/core/Utils.hh"
#include "starlight/core/Log.hh"

namespace sl {

SPIRVParser::SPIRVParser(const std::string& spirv) :
    m_compiler(
      reinterpret_cast<const u32*>(spirv.data()), spirv.size() / sizeof(u32)
    ),
    m_resources(m_compiler.get_shader_resources()) {}

std::optional<SPIRVParser::Output> SPIRVParser::process(Shader::Stage::Type stage
) && {
    if (stage == Shader::Stage::Type::vertex) processInputs();

    processUniforms();
    processSamplers();
    processPushConstants();

    return m_output;
}

static std::pair<Shader::DataType, u64> getTypeInfo(
  const spirv_cross::SPIRType& type, bool padded = true
) {
    // Align vec3 to 16 bytes (std140 / std430 rules)
    if (type.columns == 4) return { Shader::DataType::mat4, 64u };
    if (type.vecsize == 2) return { Shader::DataType::vec2, 8u };
    if (type.vecsize == 3) return { Shader::DataType::vec3, padded ? 16u : 12u };
    if (type.vecsize == 4) return { Shader::DataType::vec4, 16u };

    switch (type.basetype) {
        case spirv_cross::SPIRType::Float:
            return { Shader::DataType::f32, 4u };
        case spirv_cross::SPIRType::Int:
            return { Shader::DataType::i32, 4u };
        case spirv_cross::SPIRType::UInt:
            return { Shader::DataType::u32, 4u };
        case spirv_cross::SPIRType::Boolean:
            return { Shader::DataType::boolean, 1u };
        default:
            return { Shader::DataType::custom, 0u };
    }
};

void SPIRVParser::processInputs() {
    for (auto& res : m_resources.stage_inputs) {
        const auto [type, size] =
          getTypeInfo(m_compiler.get_type(res.type_id), false);

        m_output.attributes.push_back(Shader::InputAttribute{
          .location = m_compiler.get_decoration(res.id, spv::DecorationLocation),
          .offset   = 0u,
          .type     = type,
          .size     = size,
          .name     = res.name,
        });
    }
}

static Shader::Uniform::Scope getScope(u32 set) {
    return set == 0 ? Shader::Uniform::Scope::global : Shader::Uniform::Scope::local;
}

void SPIRVParser::processUniforms() {
    for (auto& res : m_resources.uniform_buffers) {
        auto type = m_compiler.get_type(res.base_type_id);
        for (u32 i = 0; i < type.member_types.size(); i++) {
            const auto [internalType, _] =
              getTypeInfo(m_compiler.get_type(type.member_types[i]));

            m_output.uniforms.push_back(Shader::Uniform{
              .offset = m_compiler.get_member_decoration(
                res.base_type_id, i, spv::DecorationOffset
              ),
              .binding = m_compiler.get_decoration(res.id, spv::DecorationBinding),
              .type    = internalType,
              .size    = m_compiler.get_declared_struct_member_size(type, i),
              .scope   = getScope(
                m_compiler.get_decoration(res.id, spv::DecorationDescriptorSet)
              ),
              .name = m_compiler.get_member_name(res.base_type_id, i),
            });
        }
    }
}

void SPIRVParser::processPushConstants() {
    for (auto& res : m_resources.push_constant_buffers) {
        auto type = m_compiler.get_type(res.base_type_id);

        for (u32 i = 0; i < type.member_types.size(); i++) {
            const auto [internalType, _] =
              getTypeInfo(m_compiler.get_type(type.member_types[i]));

            m_output.uniforms.push_back(Shader::Uniform{
              .offset = m_compiler.get_member_decoration(
                res.base_type_id, i, spv::DecorationOffset
              ),
              .binding = 0u,
              .type    = internalType,
              .size    = m_compiler.get_declared_struct_member_size(type, i),
              .scope   = Shader::Uniform::Scope::pushConstant,
              .name    = m_compiler.get_member_name(res.base_type_id, i),
            });
        }
    }
}

void SPIRVParser::processSamplers() {
    for (auto& res : m_resources.sampled_images) {
        auto& type = m_compiler.get_type(res.type_id);

        m_output.uniforms.push_back(Shader::Uniform{
          .offset  = m_compiler.get_decoration(res.id, spv::DecorationLocation),
          .binding = m_compiler.get_decoration(res.id, spv::DecorationBinding),
          .type    = Shader::DataType::sampler,
          .size    = type.array.empty() ? 1u : type.array[0],
          .scope   = getScope(
            m_compiler.get_decoration(res.id, spv::DecorationDescriptorSet)
          ),
          .name = res.name,
        });
    }
}

}  // namespace sl

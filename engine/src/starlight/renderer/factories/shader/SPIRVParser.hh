#pragma once

#include <optional>

#include <spirv_cross/spirv_cross.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#include "starlight/core/FileSystem.hh"
#include "starlight/renderer/gpu/ShaderProgram.hh"

namespace sl {

class SPIRVParser {
public:
    struct Output {
        std::vector<ShaderProgram::Attribute> attributes;
        std::vector<ShaderProgram::Uniform> uniforms;
    };

    explicit SPIRVParser(const std::string& spirv);

    std::optional<Output> process(ShaderProgram::Stage::Type stage) &&;

private:
    void processInputs();
    void processUniforms();
    void processSamplers();
    void processPushConstants();

    spirv_cross::Compiler m_compiler;
    spirv_cross::ShaderResources m_resources;
    Output m_output;
};

std::optional<ShaderProgram::Properties> parseShaderProgram(
  const std::string& basePath, const FileSystem& fs
);

}  // namespace sl

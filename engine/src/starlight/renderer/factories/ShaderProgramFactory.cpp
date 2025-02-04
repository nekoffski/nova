#include "ShaderProgramFactory.hh"

#include "shader/SPIRVParser.hh"

namespace sl {

static const std::vector<std::pair<std::string, ShaderProgram::Stage::Type>>
  acceptedExtensions = {
      { "frag", ShaderProgram::Stage::Type::fragment },
      { "vert", ShaderProgram::Stage::Type::vertex   },
      { "geom", ShaderProgram::Stage::Type::geometry },
      { "comp", ShaderProgram::Stage::Type::compute  },
};

static void sortMembers(ShaderProgram::Properties& props) {
    std::ranges::sort(props.attributes, [](auto& lhs, auto& rhs) -> bool {
        return lhs.location < rhs.location;
    });

    std::ranges::sort(props.uniforms, [](auto& lhs, auto& rhs) -> bool {
        if (lhs.scope == rhs.scope) {
            if (lhs.type == ShaderProgram::DataType::sampler) return false;
            if (rhs.type == ShaderProgram::DataType::sampler) return true;

            return lhs.offset < rhs.offset;
        }
        return lhs.scope < rhs.scope;
    });
}

static void logShaderProperties(const ShaderProgram::Properties& props) {
    log::debug("Shader parsed");
    log::debug("\tStages:");
    for (u32 i = 0u; auto& stage : props.stages)
        log::debug("\t\t{:02}. {}", i++, stage);

    log::debug("\tInput attributes:");
    for (u32 i = 0u; auto& attribute : props.attributes)
        log::debug("\t\t{:02}. {}", i++, attribute);

    log::debug("\tUnforms:");
    for (u32 i = 0u; auto& uniform : props.uniforms)
        log::debug("\t\t{:02}. {}", i++, uniform);
}

std::optional<ShaderProgram::Properties> parseShaderProgram(
  const std::string& basePath, const FileSystem& fs
) {
    ShaderProgram::Properties props;
    log::debug("Processing shader program: '{}'", basePath);

    for (const auto& [extension, type] : acceptedExtensions) {
        const auto stagePath = fmt::format("{}.{}.spv", basePath, extension);
        if (fs.isFile(stagePath)) {
            log::debug(
              "Found {} stage for '{}' shader, will try to process", type, basePath
            );
            const auto source = fs.readFile(stagePath);
            if (auto output = SPIRVParser{ source }.process(type); not output) {
                log::warn("Could not parse shader stage: {}", stagePath);
                return {};
            } else {
                props.stages.push_back(ShaderProgram::Stage{
                  .fullPath   = stagePath,
                  .sourceCode = source,
                  .type       = type,
                });

                std::ranges::move(
                  output->attributes, std::back_inserter(props.attributes)
                );
                std::ranges::move(
                  output->uniforms, std::back_inserter(props.uniforms)
                );
            }
        }
    }

    if (props.stages.empty()) {
        log::warn("Could not parse shader, no stages found");
        return {};
    }

    sortMembers(props);
    logShaderProperties(props);

    return props;
}

ResourceRef<ShaderProgram> ShaderProgramFactory::load(
  const std::string& name, const FileSystem& fs
) {
    auto basePath   = fmt::format("{}/{}", m_shaderProgramsPath, name);
    auto properties = parseShaderProgram(basePath, fs);

    return {};
}

ShaderProgramFactory::ShaderProgramFactory(const std::string& path
) : ResourceFactory("ShaderProgram"), m_shaderProgramsPath(path) {}

}  // namespace sl
